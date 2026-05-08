# NanoDB - Architecture & Design Document

## Executive Summary

NanoDB is a graduate-level database management system built entirely from scratch without using STL containers. This document describes the architectural decisions, implementation strategies, and performance characteristics of the system.

## 1. System Architecture

### 1.1 Layered Architecture

```
┌─────────────────────────────────────────┐
│        Query Processing Layer           │
│  (Parser, Optimizer, Executor)          │
├─────────────────────────────────────────┤
│      Execution Engine Layer             │
│  (Index Manager, Join Engine)           │
├─────────────────────────────────────────┤
│      Storage Management Layer           │
│  (Buffer Pool, Page Manager)            │
├─────────────────────────────────────────┤
│      Memory Management Layer            │
│  (Raw Pointers, Manual Allocation)      │
└─────────────────────────────────────────┘
```

## 2. Core Data Structures (All Custom Implementations)

### 2.1 Stack (LIFO)
- **File**: `include/stack.h`
- **Template**: `Stack<T>`
- **Time Complexity**: O(1) push/pop
- **Space Complexity**: O(n)
- **Usage**: Query parser for postfix evaluation

```cpp
Stack<Token> op_stack;
op_stack.push(token);
Token t = op_stack.pop();
```

### 2.2 Queue & Priority Queue (FIFO)
- **File**: `include/queue.h`
- **Templates**: `Queue<T>`, `PriorityQueue<T>`
- **Time Complexity**: O(1) enqueue/dequeue
- **Space Complexity**: O(n)
- **Usage**: Query scheduling with priority levels

```cpp
PriorityQueue<Query> pq([](const Query& a, const Query& b) {
    return a.priority > b.priority;
});
```

### 2.3 Doubly Linked List
- **File**: `include/dlinked_list.h`
- **Template**: `DoublyLinkedList<T>`
- **Time Complexity**: O(1) push/pop at front/back
- **Space Complexity**: O(n)
- **Usage**: LRU cache eviction policy

```cpp
DoublyLinkedList<int> lru_list;
lru_list.push_front(page_id);  // Mark as recently used
lru_list.pop_back();            // Evict oldest
```

### 2.4 AVL Tree (Self-Balancing BST)
- **File**: `include/avl_tree.h`
- **Template**: `AVLTree<K, V>`
- **Time Complexity**: O(log n) insert/search/delete
- **Space Complexity**: O(n)
- **Balancing**: Height-balanced with rotations
- **Usage**: Primary index for fast key lookups

```cpp
AVLTree<int, int> customer_index;  // key -> row_id
customer_index.insert(customer_id, row_index);
int row_id = -1;
customer_index.search(customer_id, row_id);
```

**Rotation Operations**:
- Left rotation: O(1)
- Right rotation: O(1)
- Left-right rotation: O(1)
- Right-left rotation: O(1)

### 2.5 Hash Map with Chaining
- **File**: `include/hash_map.h`
- **Template**: `HashMap<K, V>`
- **Time Complexity**: O(1) average, O(n) worst case
- **Space Complexity**: O(n)
- **Collision Resolution**: Chaining with doubly-linked lists
- **Load Factor**: Resizes when 50% full
- **Usage**: System catalog for table metadata

```cpp
HashMap<int, std::string> table_catalog;
table_catalog.insert(table_id, table_name);
std::string name;
table_catalog.search(table_id, name);
```

**Hash Function**:
```cpp
int hash(const K& key) {
    return ((int)key) & (capacity_ - 1);
}
```

### 2.6 Graph (For Query Optimization)
- **File**: `include/graph.h`
- **Usage**: Multi-table join optimization via Minimal Spanning Tree
- **Algorithm**: Kruskal's MST with disjoint set union

```cpp
Graph query_graph(3);  // 3 tables
query_graph.add_edge(0, 1, 100.0);  // customer -> orders
query_graph.add_edge(1, 2, 150.0);  // orders -> lineitem
Edge* mst = query_graph.find_mst(mst_count);
```

## 3. Memory Management

### 3.1 Buffer Pool (Pager)
- **File**: `include/pager.h`, `src/pager.cpp`
- **Page Size**: 4096 bytes (configurable)
- **Pool Size**: 128 pages default
- **Memory Management**: Fixed-size contiguous buffer

#### Architecture
```
┌─────────────────────────────────┐
│    Pager (Buffer Manager)       │
├─────────────────────────────────┤
│  ┌─────────────────────────────┐│
│  │  Fixed Buffer Array (128x4K)││
│  └─────────────────────────────┘│
│  ┌──────┬──────────────────────┐│
│  │ Head │ LRU Doubly-LL        ││ ← Most Recent
│  │      │ ┌────┬────┬────┐    ││
│  │      │ │ 3  │ 1  │ 42 │ ...││
│  │      │ └────┴────┴────┘    ││
│  │ Tail │                      ││ ← Least Recent
│  └──────┴──────────────────────┘│
│  ┌──────────────────────────────┐│
│  │  Hash Map (page_id -> frame) ││
│  │  Open addressing             ││
│  └──────────────────────────────┘│
└─────────────────────────────────┘
```

#### LRU Eviction Policy
1. When pool is full, evict tail of LRU list
2. If dirty, write to disk first
3. Deallocate frame and return to free pool
4. Update hash map to remove mapping

**Time Complexity**: O(1) for all operations
**Space Overhead**: O(pool_size) for metadata

### 3.2 Memory Operations
- `fetch_page(page_id)` → O(1) average, O(n) hash collision worst case
- `mark_dirty(page_id)` → O(1)
- `flush_all()` → O(pool_size)

### 3.3 Persistence
- Pages serialized as binary files: `pages/page_<id>.dat`
- Direct binary I/O: `fstream::write()` / `fstream::read()`
- Enables durability: data survives process termination

## 4. Type System

### 4.1 Polymorphic Value Types
- **File**: `include/type.h`, `src/type.cpp`
- **Base Class**: `Value` (virtual interface)
- **Derived Classes**: `IntValue`, `FloatValue`, `StringValue`

#### Interface
```cpp
virtual Value* clone() const;
virtual std::string to_string() const;
virtual int compare(const Value* other) const;  // -1, 0, 1
```

#### Cross-Type Comparisons
- Int vs Int: Direct comparison
- Float vs Float: Direct comparison
- Int vs Float: Convert Int to Double
- Float vs Int: Convert Int to Double
- String vs String: Lexicographic comparison
- Other combinations: Return 0 (no-op)

### 4.2 Field Wrapper
```cpp
struct Field {
    Value* val;
    Field();                        // NULL
    Field(Value* v);               // Takes ownership
    Field(const Field& other);     // Deep copy
    Field& operator=(const Field&);// Assignment with cleanup
    std::string to_string() const;
    int compare(const Field& o) const;
};
```

**RAII Pattern**: Automatic cleanup via destructor

## 5. Query Processing

### 5.1 Parser Architecture
- **File**: `include/query_parser.h`

#### Tokenization
1. Skip whitespace
2. Identify token type:
   - Numbers (int/float)
   - Quoted strings
   - Identifiers (column names)
   - Operators (single/double character)
   - Parentheses

```
Input:  "c_acctbal > 5000 AND c_mktsegment == \"BUILDING\""
Tokens: [IDENTIFIER:"c_acctbal"] [GREATER] [INT:5000] 
        [AND] [IDENTIFIER:"c_mktsegment"] [EQUAL] [STRING:"BUILDING"]
```

#### Shunting Yard Algorithm (Infix → Postfix)
1. Scan tokens left-to-right
2. Operands → output directly
3. Operators → push to stack based on precedence
4. Left paren → push to stack
5. Right paren → pop until matching left paren
6. At end → pop all remaining operators

**Operator Precedence** (highest to lowest):
```
6. Parentheses (implicit)
5. MULTIPLY, DIVIDE, MODULO
4. PLUS, MINUS
3. Comparison (>, <, ==, !=, >=, <=)
2. LOGICAL_AND
1. LOGICAL_OR
```

#### Postfix Evaluation
- Use stack for evaluation
- Operands → push
- Operators → pop 2, apply, push result
- Result: single value on stack

```
Infix:   3 + 4 * 2
Postfix: 3 4 2 * +
Stack:   [3] [3,4] [3,4,2] [3,8] [11]
```

### 5.2 Expression Evaluator
- **File**: `include/expr_evaluator.h`
- Evaluates postfix expressions with polymorphic operands
- Returns boolean for WHERE clause filtering

## 6. Index Management

### 6.1 AVL Tree Indexing
- Self-balancing to maintain O(log n) height
- Automatic rebalancing on insert/delete
- Balance factor = height(left) - height(right)
- Rotations maintain BST property

#### Balancing Cases
```
Left-Left:    RR rotation
Left-Right:   LR rotation
Right-Right:  LL rotation
Right-Left:   RL rotation
```

#### Update Heights
```cpp
height[node] = 1 + max(height[left], height[right])
```

### 6.2 Index Lookups
```cpp
// Sequential scan: O(n)
for (int i = 0; i < table.row_count(); ++i) {
    if (matches(table[i])) { /* found */ }
}

// Index search: O(log n)
int row_id = -1;
index.search(key, row_id);
```

**Speedup**: ~1000x for 100k records (1M operations vs ~17)

## 7. Query Optimization

### 7.1 Join Optimization via MST
- Model tables as vertices, join costs as edge weights
- Find Minimum Spanning Tree using Kruskal's algorithm
- Traverse MST to determine join order

```
Tables: customer, orders, lineitem

Cost Model:
customer ←→ orders (cost: 100)
orders ←→ lineitem (cost: 150)
customer ←→ lineitem (cost: 300)

MST:
customer -100- orders -150- lineitem
Total cost: 250

Join Order: customer → orders → lineitem
```

### 7.2 Disjoint Set Union (For Kruskal's)
- Path compression in `find()`
- Union by rank optimization
- Detects cycles in MST construction

```cpp
int find(int x) {
    if (parent[x] != x) {
        parent[x] = find(parent[x]);  // Path compression
    }
    return parent[x];
}
```

## 8. System Catalog

### 8.1 Metadata Management
- Table names → table IDs
- Column information
- Index information
- File location mappings

### 8.2 Hash Map Catalog
```cpp
HashMap<int, std::string> tables;        // id → name
HashMap<std::string, int> table_lookup;  // name → id
HashMap<int, Column*> columns;           // col_id → metadata
```

## 9. Logging System

### 9.1 Execution Logger
- **File**: `include/logger.h`
- Timestamps in milliseconds
- Structured format: `[timestamp] [LOG] message`

#### Log Events
- Page evictions (LRU policy, page ID, frame)
- Query parsing (infix → postfix)
- Index operations (sequential vs tree search timing)
- Join optimization (MST path calculation)
- Cache statistics

### 9.2 Log Output
```
[1715338800000] [LOG] Buffer pool initialized: 128 pages
[1715338800050] [LOG] Page 42 evicted via LRU, written to disk
[1715338800100] [LOG] Infix 'c_acctbal > 5000' converted to Postfix
[1715338800150] [LOG] Index search (key=1000, type=AVL): 45.23 microseconds
[1715338800200] [LOG] Multi-table join MST: customer -> orders -> lineitem
```

## 10. Complexity Analysis

### 10.1 Time Complexity Summary
| Operation | Complexity | Implementation |
|-----------|------------|-----------------|
| Page fetch | O(1) avg | Hash + LRU |
| Index insert | O(log n) | AVL tree |
| Index search | O(log n) | AVL tree |
| Sequential scan | O(n) | Array iteration |
| Join 2 tables | O(n*m) | Nested loop |
| Query parse | O(m log m) | Shunting yard |
| Hash insert | O(1) avg | Chaining |
| Hash search | O(1) avg | Chaining |

### 10.2 Space Complexity Summary
| Data Structure | Space | Notes |
|---|---|---|
| Buffer pool | O(p*s) | p pages × s bytes |
| AVL tree | O(n) | n nodes, no overhead |
| Hash map | O(n) | n entries + chains |
| DLL | O(n) | n nodes + 2 pointers |
| Stack/Queue | O(n) | n elements |

## 11. Performance Benchmarking

### 11.1 Benchmark Scenarios
1. **Index Performance**
   - Sequential: 1 million comparisons on 100k records (~500ms)
   - Index: ~17 comparisons (~0.5ms)
   - Speedup: ~1000x

2. **Join Performance**
   - 2-table join: O(n*m) = 20k × 30k = 600M operations
   - 3-table join: Optimized via MST to best ordering

3. **Cache Performance**
   - Hit rate under 50-page buffer with 5000 page accesses
   - Expected evictions: ~100 (20% of requests)

## 12. Testing Strategy

### 12.1 Smoke Tests (`main.cpp`)
- Buffer pool read/write
- Type system coercion
- Stack/Queue operations
- AVL tree insert/search
- Hash map lookups
- DLL operations
- Query parsing
- Infix to postfix conversion

### 12.2 Integration Tests (`test_runner.cpp`)
- INSERT queries
- SELECT WHERE filtering
- Index vs sequential search
- JOIN operations
- Complex expressions
- Persistence across restarts

### 12.3 Test Cases (Demo)
1. **Parser & Evaluator**: Complex WHERE clauses
2. **Index Optimizer**: Timing comparison
3. **Join Optimizer**: MST path calculation
4. **Memory Stress**: LRU eviction under constraints
5. **Priority Queue**: Query precedence
6. **Expression Trees**: Deep nesting
7. **Durability**: Persistence verification

## 13. Future Enhancements

1. **Transactions**: ACID properties, rollback
2. **Concurrency**: Locking, multi-threading
3. **Compression**: Page compression for disk
4. **Aggregations**: GROUP BY, SUM, COUNT, AVG
5. **Subqueries**: Nested SELECT support
6. **View Support**: Virtual tables
7. **Constraints**: PRIMARY KEY, FOREIGN KEY
8. **Triggers**: Custom stored procedures

## 14. References

- **Shunting Yard Algorithm**: Dijkstra's algorithm for expression parsing
- **AVL Trees**: Adelson-Velsky and Landis, 1962
- **Kruskal's Algorithm**: Joseph Kruskal, 1956
- **TPC-H Benchmark**: Transaction Processing Performance Council
- **LRU Cache**: Operating Systems memory management
