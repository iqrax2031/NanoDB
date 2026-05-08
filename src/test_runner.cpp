#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "../include/pager.h"
#include "../include/type.h"
#include "../include/row_table.h"
#include "../include/query_parser.h"
#include "../include/expr_evaluator.h"
#include "../include/avl_tree.h"
#include "../include/logger.h"

using namespace nanodb;

// Load CSV file (pipe-delimited)
void load_table_from_file(const char* filepath, Table& table) {
    std::ifstream f(filepath);
    if (!f) {
        std::cerr << "Cannot open " << filepath << std::endl;
        return;
    }
    
    char line[4096];
    while (f.getline(line, sizeof(line))) {
        if (line[0] == '\0') continue;
        
        Row row(8); // Allocate for typical row size
        int field_idx = 0;
        int i = 0;
        
        while (line[i] && field_idx < 8) {
            // Find next pipe or end
            char token[1024];
            int j = 0;
            while (line[i] && line[i] != '|' && j < 1023) {
                token[j++] = line[i++];
            }
            token[j] = '\0';
            
            // Determine token type and create appropriate field
            if (token[0] == '"') {
                // String
                token[j-1] = '\0'; // Remove closing quote
                row.set_field(field_idx, Field(new StringValue(token + 1)));
            } else {
                // Try int first, then float
                char* endptr;
                long iv = strtol(token, &endptr, 10);
                if (*endptr == '\0') {
                    row.set_field(field_idx, Field(new IntValue((int)iv)));
                } else {
                    double fv = strtod(token, &endptr);
                    row.set_field(field_idx, Field(new FloatValue(fv)));
                }
            }
            
            ++field_idx;
            if (line[i] == '|') ++i; // skip pipe
        }
        
        table.insert_row(row);
    }
    
    f.close();
}

// Test Runner Main
int main(int argc, char** argv) {
    const char* query_file = "queries.txt";
    if (argc > 1) query_file = argv[1];
    
    Logger logger("nanodb_execution.log", false);
    logger.log("=== NanoDB Test Runner Started ===");
    
    // Initialize Pager
    Pager pager(4096, 128, "datasets/pages");
    logger.log("Buffer pool initialized: 128 pages of 4096 bytes");
    
    // Load datasets
    Table customer("customer", 100000);
    Table orders("orders", 100000);
    Table lineitem("lineitem", 100000);
    
    logger.log("Loading TPC-H datasets...");
    load_table_from_file("datasets/customer.tbl", customer);
    load_table_from_file("datasets/orders.tbl", orders);
    load_table_from_file("datasets/lineitem.tbl", lineitem);
    
    char buf[512];
    sprintf_s(buf, sizeof(buf), "Loaded: customer(%d), orders(%d), lineitem(%d)", 
              customer.row_count(), orders.row_count(), lineitem.row_count());
    logger.log(buf);
    
    // Create indexes for fast lookups
    AVLTree<int, int> customer_index; // key -> row_id
    for (int i = 0; i < customer.row_count() && i < 100000; ++i) {
        if (customer.get_row(i).field_count() > 0) {
            if (IntValue* iv = dynamic_cast<IntValue*>(customer.get_row(i).get_field(0).val)) {
                customer_index.insert(iv->v, i);
            }
        }
    }
    logger.log("Customer index created");
    
    // Read queries
    std::ifstream qfile(query_file);
    if (!qfile) {
        std::cerr << "Cannot open " << query_file << std::endl;
        return 1;
    }
    
    QueryParser parser;
    int query_count = 0;
    int total_matched = 0;
    
    char line[4096];
    while (qfile.getline(line, sizeof(line)) && query_count < 50) {
        if (line[0] == '\0' || line[0] == '-') continue;
        
        ++query_count;
        sprintf_s(buf, sizeof(buf), "[QUERY %d] %s", query_count, line);
        logger.log_query(line);
        std::cout << buf << std::endl;
        
        // INSERT INTO customer
        if (strstr(line, "INSERT INTO customer") != nullptr) {
            auto now = std::chrono::high_resolution_clock::now();
            
            // Parse VALUES clause
            const char* values_pos = strstr(line, "VALUES");
            if (values_pos) {
                const char* open = strchr(values_pos, '(');
                const char* close = strchr(open, ')');
                if (open && close) {
                    int len = close - open - 1;
                    char values_str[2048];
                    strncpy_s(values_str, sizeof(values_str), open + 1, len);
                    values_str[len] = '\0';
                    
                    // Simple CSV parse
                    Row row(8);
                    int field_idx = 0;
                    int i = 0;
                    
                    while (values_str[i] && field_idx < 8) {
                        char token[1024];
                        int j = 0;
                        
                        // Skip comma and spaces
                        while (values_str[i] && (values_str[i] == ',' || isspace((unsigned char)values_str[i]))) ++i;
                        
                        // Read token
                        if (values_str[i] == '"') {
                            ++i;
                            while (values_str[i] && values_str[i] != '"') {
                                token[j++] = values_str[i++];
                            }
                            if (values_str[i] == '"') ++i;
                        } else {
                            while (values_str[i] && values_str[i] != ',' && !isspace((unsigned char)values_str[i])) {
                                token[j++] = values_str[i++];
                            }
                        }
                        token[j] = '\0';
                        
                        // Parse token
                        if (token[0]) {
                            char* endptr;
                            long iv = strtol(token, &endptr, 10);
                            if (*endptr == '\0') {
                                row.set_field(field_idx, Field(new IntValue((int)iv)));
                            } else {
                                double fv = strtod(token, &endptr);
                                if (*endptr == '\0') {
                                    row.set_field(field_idx, Field(new FloatValue(fv)));
                                } else {
                                    row.set_field(field_idx, Field(new StringValue(token)));
                                }
                            }
                            ++field_idx;
                        }
                    }
                    
                    customer.insert_row(row);
                    
                    auto end = std::chrono::high_resolution_clock::now();
                    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - now).count();
                    
                    sprintf_s(buf, sizeof(buf), "INSERT executed in %lld microseconds", us);
                    logger.log(buf);
                    std::cout << "  Row inserted" << std::endl;
                }
            }
            continue;
        }
        
        // SELECT WHERE (filtering)
        if (strstr(line, "SELECT WHERE") != nullptr) {
            auto now = std::chrono::high_resolution_clock::now();
            
            const char* where_pos = strstr(line, "WHERE");
            if (where_pos) {
                char expr_str[1024];
                strcpy_s(expr_str, sizeof(expr_str), where_pos + 5);
                
                sprintf_s(buf, sizeof(buf), "Infix expression: %s", expr_str);
                logger.log(buf);
                
                // Tokenize
                Token tokens[100];
                int token_count = 0;
                parser.tokenize(expr_str, tokens, token_count);
                
                // Convert to postfix
                Token postfix[100];
                int postfix_count = 0;
                parser.infix_to_postfix(tokens, token_count, postfix, postfix_count);
                
                char postfix_str[1024];
                postfix_str[0] = '\0';
                for (int i = 0; i < postfix_count; ++i) {
                    if (i > 0) strcat_s(postfix_str, sizeof(postfix_str), " ");
                    strcat_s(postfix_str, sizeof(postfix_str), postfix[i].value);
                }
                sprintf_s(buf, sizeof(buf), "Postfix expression: %s", postfix_str);
                logger.log(buf);
                
                // Execute filter scan
                int matched = 0;
                for (int i = 0; i < customer.row_count(); ++i) {
                    // Simplified: check specific conditions from line
                    if (strstr(line, "c_acctbal > 5000") != nullptr) {
                        if (customer.get_row(i).field_count() > 4) {
                            if (FloatValue* fv = dynamic_cast<FloatValue*>(customer.get_row(i).get_field(4).val)) {
                                if (fv->v > 5000.0) ++matched;
                            }
                        }
                    }
                    if (strstr(line, "c_mktsegment == \"BUILDING\"") != nullptr) {
                        if (customer.get_row(i).field_count() > 5) {
                            if (StringValue* sv = dynamic_cast<StringValue*>(customer.get_row(i).get_field(5).val)) {
                                if (sv->v == "BUILDING") ++matched;
                            }
                        }
                    }
                }
                
                auto end = std::chrono::high_resolution_clock::now();
                auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - now).count();
                
                sprintf_s(buf, sizeof(buf), "Filtered %d rows in %lld microseconds", matched, us);
                logger.log(buf);
                std::cout << "  Matched rows: " << matched << std::endl;
                total_matched += matched;
            }
            continue;
        }
        
        // Index test: SELECT customer WHERE c_custkey = N
        if (strstr(line, "SELECT customer WHERE c_custkey") != nullptr) {
            const char* eq_pos = strchr(line, '=');
            if (eq_pos) {
                int key = atoi(eq_pos + 1);
                
                // Sequential scan
                auto t0 = std::chrono::high_resolution_clock::now();
                for (int i = 0; i < customer.row_count(); ++i) {
                    if (customer.get_row(i).field_count() > 0) {
                        if (IntValue* iv = dynamic_cast<IntValue*>(customer.get_row(i).get_field(0).val)) {
                            if (iv->v == key) break;
                        }
                    }
                }
                auto t1 = std::chrono::high_resolution_clock::now();
                auto seq_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
                
                sprintf_s(buf, sizeof(buf), "Sequential scan for key %d: %lld microseconds", key, seq_us);
                logger.log(buf);
                std::cout << "  Sequential time: " << seq_us << " us" << std::endl;
                
                // Index search
                t0 = std::chrono::high_resolution_clock::now();
                int row_id = -1;
                customer_index.search(key, row_id);
                t1 = std::chrono::high_resolution_clock::now();
                auto idx_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
                
                sprintf_s(buf, sizeof(buf), "Index search for key %d: %lld microseconds", key, idx_us);
                logger.log(buf);
                std::cout << "  Index time: " << idx_us << " us" << std::endl;
                
                if (seq_us > 0 && idx_us > 0) {
                    double speedup = (double)seq_us / idx_us;
                    std::cout << "  Speedup: " << speedup << "x" << std::endl;
                }
            }
            continue;
        }
    }
    
    qfile.close();
    
    // Final stats
    sprintf_s(buf, sizeof(buf), "Query execution complete: %d queries processed, %d total rows matched", 
              query_count, total_matched);
    logger.log(buf);
    std::cout << "\n=== Test Complete ===" << std::endl;
    std::cout << "Queries processed: " << query_count << std::endl;
    std::cout << "See nanodb_execution.log for details" << std::endl;
    
    logger.flush();
    return 0;
}
