#!/bin/bash
# =====================================================================
# NanoDB Build Script for Linux/macOS/WSL
# =====================================================================

set -e  # Exit on error

echo ""
echo "===================================================================="
echo "  NanoDB - Database Engine Build Script"
echo "===================================================================="
echo ""

# Check for C++ compiler
if ! command -v g++ &> /dev/null; then
    if ! command -v clang++ &> /dev/null; then
        echo "Error: Neither g++ nor clang++ found in PATH"
        echo "Please install GCC or Clang:"
        echo "  Ubuntu/Debian: sudo apt-get install build-essential"
        echo "  macOS: brew install gcc"
        echo "  CentOS/RHEL: sudo yum install gcc-c++"
        exit 1
    else
        CXX=clang++
    fi
else
    CXX=g++
fi

echo "[*] Using compiler: $CXX"
$CXX --version | head -1

# Create build directories
mkdir -p build/obj
mkdir -p bin
mkdir -p datasets

echo "[*] Created build directories"

# Compile flags
CXXFLAGS="-std=c++17 -Wall -Wextra -O2 -Iinclude"

# Compile source files
echo ""
echo "[*] Compiling source files..."

echo "   - Compiling pager.cpp..."
$CXX $CXXFLAGS -c src/pager.cpp -o build/obj/pager.o
echo "     [OK] pager.o"

echo "   - Compiling type.cpp..."
$CXX $CXXFLAGS -c src/type.cpp -o build/obj/type.o
echo "     [OK] type.o"

echo "   - Compiling main.cpp..."
$CXX $CXXFLAGS -c src/main.cpp -o build/obj/main.o
echo "     [OK] main.o"

echo "   - Compiling test_runner.cpp..."
$CXX $CXXFLAGS -c src/test_runner.cpp -o build/obj/test_runner.o
echo "     [OK] test_runner.o"

# Link executable
echo ""
echo "[*] Linking executables..."
echo "   - Linking nanodb..."
$CXX build/obj/pager.o build/obj/type.o build/obj/main.o build/obj/test_runner.o \
    -o bin/nanodb
echo "     [OK] nanodb"

# Set executable permissions
chmod +x bin/nanodb

# Summary
echo ""
echo "===================================================================="
echo "  Build Complete!"
echo "===================================================================="
echo ""
echo "Executable created: bin/nanodb"
echo ""
echo "Next steps:"
echo "   1. Place TPC-H data files in datasets/ directory:"
echo "      - customer.tbl"
echo "      - orders.tbl"
echo "      - lineitem.tbl"
echo ""
echo "   2. Run the smoke test:"
echo "      ./bin/nanodb"
echo ""
echo "   3. Run with queries:"
echo "      ./bin/nanodb queries.txt"
echo ""
