# ============================================================================
# zstr.h - Modern Single-Header String Library for C
# ============================================================================

# Configuration
BUNDLER     = z-core/zbundler.py
SRC_CORE    = src/zstr.c
DIST_HEADER = zstr.h

# Compiler Settings
CC          = gcc
CFLAGS      = -O3 -march=native -Wall -Wextra
LDFLAGS     =

# Optional: Enable mimalloc for better performance
# Set USE_MIMALLOC=1 to enable (e.g., make USE_MIMALLOC=1 bench_c)
USE_MIMALLOC ?= 0
ifeq ($(USE_MIMALLOC), 1)
    CFLAGS  += -DUSE_MIMALLOC
    LDFLAGS += -lmimalloc
endif

# Optional: Enable OpenMP for parallel operations
# Set USE_OPENMP=1 to enable (e.g., make USE_OPENMP=1 bench_advanced)
USE_OPENMP ?= 0
ifeq ($(USE_OPENMP), 1)
    CFLAGS  += -fopenmp
    LDFLAGS += -fopenmp
endif

# Standard Lua (Default: 5.4)
LUA_STD_VER = lua5.4
LUA_STD_INC = /usr/include/$(LUA_STD_VER)
LUA_STD_LIB = $(LUA_STD_VER)

# LuaJIT (Manual command settings)
LUA_JIT_INC = /usr/include/luajit-2.1
LUA_JIT_LIB = luajit-5.1

# Source/Output settings
LUA_SRC = bindings/lua/zstr_module.c
LUA_OUT = zstr.so

BENCH_DIR = benchmarks/c
SDS_URL   = https://raw.githubusercontent.com/antirez/sds/master

.DEFAULT_GOAL := help

# ============================================================================
# Help Target
# ============================================================================

.PHONY: help
help:
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║                    zstr.h - Makefile Targets                       ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Build Targets:"
	@echo "  make bundle         - Bundle the C library into single header"
	@echo "  make lua            - Build Lua 5.4 bindings"
	@echo "  make luajit         - Build LuaJIT bindings"
	@echo "  make all            - Build everything (bundle + lua)"
	@echo ""
	@echo "Benchmark Targets:"
	@echo "  make bench          - Run all benchmarks"
	@echo "  make bench_c        - Run C benchmarks (zstr vs Malloc)"
	@echo "  make bench_sds      - Run SDS comparison benchmarks"
	@echo "  make bench_optimized - Run optimization benchmarks (alignment, I/O, cache)"
	@echo "  make bench_advanced - Run advanced benchmarks (SIMD, Prefetch, OpenMP)"
	@echo "  make bench_comprehensive - Run comprehensive benchmark suite"
	@echo "  make bench_lua      - Run Lua benchmarks"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean          - Remove build artifacts"
	@echo "  make init           - Initialize git submodules"
	@echo "  make help           - Show this help message"
	@echo ""
	@echo "Options:"
	@echo "  USE_MIMALLOC=1      - Enable mimalloc allocator for benchmarks"
	@echo "                        Example: make USE_MIMALLOC=1 bench_c"
	@echo "  USE_OPENMP=1        - Enable OpenMP parallelization"
	@echo "                        Example: make USE_OPENMP=1 USE_MIMALLOC=1 bench_advanced"
	@echo ""
	@echo "Quick Start:"
	@echo "  1. make init        # Initialize submodules"
	@echo "  2. make bundle      # Generate zstr.h"
	@echo "  3. make bench       # Run benchmarks"
	@echo ""

all: bundle lua

# Bundle the core C library.
bundle:
	@echo "Bundling $(DIST_HEADER)..."
	@if [ ! -f $(BUNDLER) ]; then \
		echo "Initializing git submodules..."; \
		git submodule update --init --recursive; \
	fi
	python3 $(BUNDLER) $(SRC_CORE) $(DIST_HEADER)

# Build Lua Bindings.
lua: LUA_CURRENT_INC = $(LUA_STD_INC)
lua: LUA_CURRENT_LIB = $(LUA_STD_LIB)
lua: build_msg = "Building Standard Lua 5.4 binding..."
lua: bundle $(LUA_SRC) build_shared

luajit: LUA_CURRENT_INC = $(LUA_JIT_INC)
luajit: LUA_CURRENT_LIB = $(LUA_JIT_LIB)
luajit: build_msg = "Building LuaJIT binding..."
luajit: bundle $(LUA_SRC) build_shared

build_shared:
	@echo $(build_msg)
	$(CC) -O3 -shared -fPIC -o $(LUA_OUT) $(LUA_SRC) -I. -I$(LUA_CURRENT_INC) -l$(LUA_CURRENT_LIB)

# Helper: Download SDS source only if missing
download_sds:
	@mkdir -p $(BENCH_DIR)
	@if [ ! -f $(BENCH_DIR)/sds.h ]; then \
		echo "Downloading SDS for benchmarks..."; \
		wget -q $(SDS_URL)/sds.h -O $(BENCH_DIR)/sds.h; \
		wget -q $(SDS_URL)/sds.c -O $(BENCH_DIR)/sds.c; \
		wget -q $(SDS_URL)/sdsalloc.h -O $(BENCH_DIR)/sdsalloc.h; \
	fi

# C Benchmark: zstr vs Raw Malloc.
bench_c: bundle
	@echo "=> Compiling C benchmarks (zstr vs Malloc)"
ifeq ($(USE_MIMALLOC), 1)
	@echo "   Using mimalloc allocator"
endif
	$(CC) $(CFLAGS) -o $(BENCH_DIR)/bench_c $(BENCH_DIR)/main.c -I. $(LDFLAGS)
	@echo "Running..."
	./$(BENCH_DIR)/bench_c

# C Benchmark: zstr vs SDS.
bench_sds: bundle download_sds
	@echo "=> Compiling SDS benchmarks"
ifeq ($(USE_MIMALLOC), 1)
	@echo "   Using mimalloc allocator"
endif
	$(CC) $(CFLAGS) -o $(BENCH_DIR)/bench_sds $(BENCH_DIR)/bench_sds.c $(BENCH_DIR)/sds.c -I. -I$(BENCH_DIR) $(LDFLAGS)
	@echo "Running..."
	./$(BENCH_DIR)/bench_sds

# Lua Benchmark: zstr vs Native Lua.
bench_lua: luajit
	@echo "=> Running Lua Benchmarks"
	LUA_CPATH="./?.so;;" luajit benchmarks/lua/general.lua
	LUA_CPATH="./?.so;;" luajit benchmarks/lua/jit.lua
	LUA_CPATH="./?.so;;" luajit benchmarks/lua/bulk.lua

# Enhanced optimization benchmark
bench_optimized: bundle
	@echo "=> Compiling enhanced optimization benchmarks"
ifeq ($(USE_MIMALLOC), 1)
	@echo "   Using mimalloc allocator"
endif
	$(CC) $(CFLAGS) -o $(BENCH_DIR)/bench_optimized $(BENCH_DIR)/bench_optimized.c -I. $(LDFLAGS)
	@echo "Running..."
	./$(BENCH_DIR)/bench_optimized

# Advanced optimization benchmark (SIMD, Prefetch, OpenMP)
bench_advanced: bundle
	@echo "=> Compiling advanced optimization benchmarks"
ifeq ($(USE_MIMALLOC), 1)
	@echo "   Using mimalloc allocator"
endif
ifeq ($(USE_OPENMP), 1)
	@echo "   Using OpenMP parallelization"
endif
	$(CC) $(CFLAGS) -o $(BENCH_DIR)/bench_advanced $(BENCH_DIR)/bench_advanced.c -I. $(LDFLAGS)
	@echo "Running..."
	./$(BENCH_DIR)/bench_advanced

# Comprehensive benchmark suite (all optimizations)
bench_comprehensive: bundle
	@echo "=> Compiling comprehensive benchmark suite"
ifeq ($(USE_MIMALLOC), 1)
	@echo "   Using mimalloc allocator"
endif
ifeq ($(USE_OPENMP), 1)
	@echo "   Using OpenMP parallelization"
endif
	$(CC) $(CFLAGS) -o $(BENCH_DIR)/bench_comprehensive $(BENCH_DIR)/bench_comprehensive.c -I. $(LDFLAGS)
	@echo "Running..."
	./$(BENCH_DIR)/bench_comprehensive

bench: bench_c bench_sds bench_lua bench_optimized bench_advanced bench_comprehensive


clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(LUA_OUT)
	@rm -f $(BENCH_DIR)/bench_c $(BENCH_DIR)/bench_sds $(BENCH_DIR)/bench_optimized $(BENCH_DIR)/bench_advanced $(BENCH_DIR)/bench_comprehensive
	@echo "Done. (SDS files kept - use 'rm -f $(BENCH_DIR)/sds*' to remove)"

init:
	git submodule update --init --recursive

.PHONY: all bundle lua luajit build_shared bench bench_c bench_sds bench_lua bench_advanced bench_comprehensive download_sds clean init
