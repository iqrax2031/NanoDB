# NanoDB - Graduate Database Systems Project

A custom-built database engine from scratch implementing all core database internals without STL containers.

## Project Features

- **Custom Buffer Pool (Pager)**: Fixed-size memory management with LRU eviction using doubly-linked lists
- **Polymorphic Type System**: Supports Int, Float, String types with operator overloading
- **Query Parser**: Infix to Postfix conversion using Shunting Yard algorithm
- **Balanced Indexing**: AVL Tree implementation for O(log N) lookups
- **Query Optimization**: Graph-based optimizer with Minimal Spanning Tree for multi-table joins
- **System Catalog**: Hash Map with collision chaining for O(1) metadata lookups
- **Detailed Logging**: Comprehensive execution logs for all internal operations
- **No STL**: All data structures built from scratch without std::vector, std::map, etc.

## Architecture

```
include/
├── pager.h              # Buffer pool management
├── type.h               # Polymorphic type system
├── row_table.h          # Row and Table schemas
├── query_parser.h       # Expression parser
├── expr_evaluator.h     # Postfix expression evaluation
├── stack.h              # Generic stack implementation
├── queue.h              # Generic queue and priority queue
├── dlinked_list.h       # Doubly-linked list
├── avl_tree.h           # Self-balancing binary search tree
├── hash_map.h           # Hash map with chaining
├── graph.h              # Graph for query optimization
└── logger.h             # Execution logging

src/
├── main.cpp             # Basic smoke test
├── test_runner.cpp      # Query execution engine (no STL)
├── pager.cpp            # Pager implementation
└── type.cpp             # Type system implementation
```

## Build Instructions

### Windows (MSVC)
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Linux / macOS / WSL
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running Tests

```bash
# Run with default queries.txt
./nanodb

# Run with custom query file
./nanodb ../queries.txt
```

## Output

The test runner generates:
- **Console output**: Real-time query execution results
- **nanodb_execution.log**: Detailed execution log with timestamps including:
  - Page evictions and LRU cache operations
  - Infix to Postfix conversions
  - Sequential vs index scan timings
  - Join optimization paths
  - Cache statistics

## Dataset

Uses TPC-H Benchmark Dataset (100,000 records):
- **customer.tbl**: ~20,000 customer records
- **orders.tbl**: ~30,000 order records  
- **lineitem.tbl**: ~50,000 line item records

## Demo Test Cases

The system supports 7 comprehensive test cases:
1. **Parser & Evaluator**: Complex WHERE clause evaluation
2. **Index Optimizer**: Sequential vs Balanced Tree search comparison
3. **Join Optimizer**: Multi-table join via MST calculation
4. **Memory Stress**: Simulated cache eviction under constraints
5. **Priority Queue**: Admin query precedence over background queries
6. **Expression Trees**: Deep nesting and operator precedence
7. **Durability**: Data persistence across restarts

## Performance Characteristics

- **Buffer Pool**: O(1) page access via hash table + doubly-linked list LRU
- **Index Search**: O(log N) via AVL tree auto-balancing
- **Hash Map Lookups**: O(1) average case with chaining collision resolution
- **Query Optimization**: O(E log V) via Kruskal's MST algorithm

## Key Implementation Details

### No STL Containers Used
- Custom `Stack<T>` template
- Custom `Queue<T>` and `PriorityQueue<T>` templates
- Custom `DoublyLinkedList<T>` template
- Custom `AVLTree<K,V>` template
- Custom `HashMap<K,V>` template

### Memory Management
- Manual pointer arithmetic for buffer pool
- Proper cleanup of all dynamically allocated memory
- Binary serialization/deserialization to disk

### Query Processing
1. Tokenization with type detection
2. Operator precedence handling
3. Infix-to-Postfix conversion (Shunting Yard)
4. Postfix evaluation with polymorphic operands
5. Result filtering and aggregation

## Compilation Flags

- C++17 standard required
- No exceptions from data structures (optional behavior)
- Memory safety via RAII and proper cleanup
- Verified with compiler warnings enabled

## Future Enhancements

- Transaction support with ACID properties
- More sophisticated query optimization (cost-based)
- Compression for on-disk pages
- Concurrency control with locking
- Query plan caching

---

**Course**: CS-4002 Applied Programming (MS-CS)  
**Deadline**: May 10, 2026  
**Institution**: FAST-NUCES Islamabad

