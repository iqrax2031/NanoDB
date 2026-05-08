# NanoDB Build Guide

## Quick Start

### Windows (MSVC)
```batch
build.bat
```

### Linux/macOS/WSL (GCC/Clang)
```bash
bash build.sh
```

### Cross-Platform (CMake)
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## System Requirements

### Windows
- Visual Studio 2019 or later (MSVC compiler)
  - Community Edition is sufficient
  - Install "Desktop development with C++"
- OR Windows Subsystem for Linux (WSL)

### Linux/macOS
- GCC 9.0+ or Clang 10.0+
- GNU Make 4.0+
- CMake 3.10+ (optional)

#### Installation
**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential git
```

**macOS:**
```bash
brew install gcc
# or use system clang
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
```

## Build Methods

### Method 1: Automated Scripts (Recommended)

#### Windows (MSVC)
```batch
cd c:\workspace
build.bat
```
- Automatically finds MSVC compiler
- Compiles all sources
- Links executable to `bin\nanodb.exe`

#### Linux/macOS/WSL
```bash
cd /path/to/workspace
bash build.sh
chmod +x bin/nanodb
```
- Automatically finds GCC or Clang
- Compiles all sources
- Links executable to `bin/nanodb`

### Method 2: Makefile

```bash
# Build
make

# Clean
make clean

# Rebuild
make distclean && make

# Run
make run

# Debug build
make debug
```

**Supported make variants:**
- GNU Make (recommended)
- BSD Make
- nmake (Windows)

### Method 3: CMake

```bash
# Configure
mkdir build
cd build
cmake ..

# Build
cmake --build .

# Or with make
make

# Or with MSVC
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Method 4: Manual Compilation

**Windows (MSVC - Developer Command Prompt):**
```batch
cl /std:c++17 /W3 /EHsc /O2 /Iinclude /c src/pager.cpp
cl /std:c++17 /W3 /EHsc /O2 /Iinclude /c src/type.cpp
cl /std:c++17 /W3 /EHsc /O2 /Iinclude /c src/main.cpp
cl /std:c++17 /W3 /EHsc /O2 /Iinclude /c src/test_runner.cpp

link pager.obj type.obj main.obj test_runner.obj /OUT:nanodb.exe
```

**Linux/macOS:**
```bash
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude -c src/pager.cpp
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude -c src/type.cpp
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude -c src/main.cpp
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude -c src/test_runner.cpp

g++ pager.o type.o main.o test_runner.o -o nanodb
```

## Compiler Flags Explanation

| Flag | Purpose |
|------|---------|
| `-std=c++17` | Use C++17 standard |
| `-Wall` | Enable all warnings |
| `-Wextra` | Enable extra warnings |
| `-O2` | Optimize for performance |
| `-Iinclude` | Include directory |
| `/EHsc` | MSVC exception handling |
| `/W3` | MSVC warning level |
| `-g` | Debug symbols (optional) |
| `-O0` | No optimization (debug) |

## Directory Structure After Build

```
workspace/
├── bin/
│   └── nanodb          (or nanodb.exe on Windows)
├── build/
│   ├── obj/
│   │   ├── pager.o
│   │   ├── type.o
│   │   ├── main.o
│   │   └── test_runner.o
│   └── CMakeFiles/     (if using CMake)
├── datasets/
│   ├── customer.tbl    (add manually)
│   ├── orders.tbl      (add manually)
│   ├── lineitem.tbl    (add manually)
│   └── pages/          (created at runtime)
├── include/
├── src/
└── queries.txt
```

## Setup

### 1. Clone/Prepare Workspace
```bash
cd /path/to/workspace
git init
git add .
git commit -m "Initial NanoDB implementation"
```

### 2. Prepare Datasets
Download or generate TPC-H data (scale factor 0.1):
```bash
# Create sample customer.tbl
mkdir -p datasets
cat > datasets/customer.tbl << 'EOF'
1|Customer1|Address1|1|1000.00|BUILDING|comment
2|Customer2|Address2|2|2000.00|AUTOMOBILE|comment
...
EOF

# Similar for orders.tbl and lineitem.tbl
```

### 3. Build Project
```bash
# Windows
build.bat

# Linux/macOS
bash build.sh
```

### 4. Verify Installation
```bash
# Run smoke test
./bin/nanodb          # Unix
bin\nanodb.exe        # Windows
```

Expected output:
```
==================================================================
  NanoDB - Database Engine Smoke Test Suite
==================================================================

TEST 1: Buffer Pool (Pager) with LRU Cache
✓ Pager initialized: 8 pages of 4096 bytes
...
```

## Troubleshooting

### Error: "Compiler not found"

**Windows:**
- Open "Developer Command Prompt for Visual Studio"
- Or install Visual Studio with C++ tools
- Or use Windows Subsystem for Linux (WSL)

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# Check installation
g++ --version
```

**macOS:**
```bash
# Install Command Line Tools
xcode-select --install

# Or use Homebrew
brew install gcc
```

### Error: "C++ standard not supported"
- Upgrade compiler to C++17 capable version
- GCC 7.0+, Clang 5.0+, MSVC 2017+

### Error: "Header files not found"
- Ensure `include/` directory exists
- Check `-Iinclude` flag is present

### Build succeeds but execution fails
- Ensure `datasets/` directory exists
- Add required data files (customer.tbl, orders.tbl, lineitem.tbl)
- Check permissions: `chmod +x bin/nanodb` (Unix)

## Advanced Options

### Optimization Levels
```bash
# Development (fast compile, debug symbols)
make debug

# Production (slow compile, fast runtime)
make clean && make CXXFLAGS="-O3 -march=native"
```

### Compiler Alternatives

**Using Clang on Linux:**
```bash
CXX=clang++ make
```

**Using Intel Compiler:**
```bash
CXX=icpc make
```

### Static Analysis
```bash
# With GCC warnings
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion ...

# With Clang static analyzer
scan-build make

# With GCC Analyzer (GCC 10+)
g++ -std=c++17 -fanalyzer ...
```

### Memory Debugging
```bash
# Compile with debug symbols
make debug

# Run with Valgrind (Linux)
valgrind --leak-check=full ./bin/nanodb

# Run with AddressSanitizer
make clean && make CXXFLAGS="-fsanitize=address -g"
./bin/nanodb
```

## Continuous Integration

### GitHub Actions Example
```yaml
name: Build NanoDB
on: [push, pull_request]
jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    steps:
      - uses: actions/checkout@v2
      - name: Build on ${{ matrix.os }}
        run: |
          mkdir build && cd build
          cmake ..
          cmake --build . --config Release
```

## Performance Tuning

### Compiler Optimizations
```bash
# High performance build
g++ -O3 -march=native -flto -DNDEBUG src/*.cpp -o nanodb

# For testing (faster compile)
g++ -O1 -g src/*.cpp -o nanodb
```

### Processor-Specific Optimizations
```bash
# Intel processors
g++ -O3 -march=native -mfma ...

# AMD processors
g++ -O3 -march=znver2 ...

# Generic (portable)
g++ -O3 -march=generic ...
```

## Documentation

- **DESIGN.md** - Comprehensive architecture document
- **README.md** - Project overview and usage
- **IMPLEMENTATION.md** - Detailed implementation notes
- **include/*.h** - Well-documented header files

## Version Control

```bash
# Initialize repository
git init
git add .
git commit -m "Initial commit: NanoDB implementation"

# Create meaningful commits
git add include/stack.h
git commit -m "feat: Implement generic Stack<T> template"

git add include/avl_tree.h
git commit -m "feat: Implement AVL tree self-balancing BST"
```

## Submission

### Required Files for Submission
1. **Complete source code** (.h and .cpp files)
2. **Build script** (Makefile or CMakeLists.txt)
3. **Compiled executable** (optional)
4. **Dataset files** (customer.tbl, orders.tbl, lineitem.tbl)
5. **README.md** with build instructions
6. **DESIGN.md** with architecture
7. **queries.txt** with 50 queries
8. **nanodb_execution.log** (after running tests)

### Archive Submission
```bash
# Create ZIP archive
zip -r NanoDB.zip \
    src/ include/ \
    Makefile CMakeLists.txt \
    README.md DESIGN.md \
    queries.txt datasets/customer.tbl datasets/orders.tbl datasets/lineitem.tbl
```

## Support

For compilation issues:
1. Check compiler version: `g++ --version`
2. Verify C++17 support
3. Check include paths: `-Iinclude`
4. Enable verbose output: `make VERBOSE=1`
5. Review full error messages

---

**Last Updated**: May 2026
**NanoDB Version**: 1.0
**C++ Standard**: C++17
