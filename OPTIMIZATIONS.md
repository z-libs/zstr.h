# zstr.h Optimization Guide

This document describes the performance optimizations implemented in zstr.h and how to take advantage of them.

> **Note**: For advanced optimizations including SIMD, Prefetch, and OpenMP, see [ADVANCED_OPTIMIZATIONS.md](ADVANCED_OPTIMIZATIONS.md)

## Overview

The following optimizations have been implemented:

1. **mimalloc Integration** - Optional use of Microsoft's high-performance allocator
2. **Cache-Aligned Data Structures** - Optimized memory layout for better cache utilization
3. **Optimized File I/O** - Chunked reading with better buffer alignment
4. **Enhanced Benchmarking** - Comprehensive performance testing suite
5. **SIMD Optimizations** - Vectorized string operations (AVX2/SSE2) - See [ADVANCED_OPTIMIZATIONS.md](ADVANCED_OPTIMIZATIONS.md)
6. **Prefetch Hints** - Cache prefetching for bulk operations - See [ADVANCED_OPTIMIZATIONS.md](ADVANCED_OPTIMIZATIONS.md)
7. **OpenMP Parallelization** - Multi-threaded bulk processing - See [ADVANCED_OPTIMIZATIONS.md](ADVANCED_OPTIMIZATIONS.md)

## 1. mimalloc Integration

[mimalloc](https://github.com/microsoft/mimalloc) is a high-performance memory allocator from Microsoft Research that provides:

- Better performance for frequent allocations/deallocations
- Lower memory fragmentation
- Thread-safe operations with minimal contention
- Excellent cache locality

### How to Enable

#### Option 1: Compile-time Flag (Recommended)

Build your project with the `USE_MIMALLOC` flag:

```bash
# For benchmarks
make USE_MIMALLOC=1 bench_c
make USE_MIMALLOC=1 bench_optimized

# For your own project
gcc -O3 -DUSE_MIMALLOC yourcode.c -lmimalloc
```

#### Option 2: Manual Integration

Before including `zstr.h`, define the allocator macros:

```c
#include <mimalloc.h>

#define Z_MALLOC(sz)      mi_malloc(sz)
#define Z_CALLOC(n, sz)   mi_calloc(n, sz)
#define Z_REALLOC(p, sz)  mi_realloc(p, sz)
#define Z_FREE(p)         mi_free(p)

#include "zstr.h"
```

### Installing mimalloc

#### Ubuntu/Debian
```bash
sudo apt-get install libmimalloc-dev
```

#### Other Systems
Download from: https://github.com/microsoft/mimalloc

### Performance Impact

Benchmark results show measurable improvements with mimalloc:

**Standard Allocator:**
- SSO Sequential: 0.72 ns/op
- File Read: 11,526 MB/s
- Small String Ops: 0.40 ns/op
- Large String Ops: 0.27 µs/op

**With mimalloc:**
- SSO Sequential: 0.62 ns/op (14% faster)
- File Read: 12,832 MB/s (11% faster)
- Small String Ops: 0.31 ns/op (23% faster)
- Large String Ops: 0.24 µs/op (11% faster)

## 2. Cache-Aligned Data Structures

The `zstr` struct is designed for optimal cache performance:

```c
struct zstr {
    uint8_t is_long;     // 1 byte - frequently accessed flag
    char _pad[7];        // 7 bytes - explicit alignment padding
    union {              // 24 bytes - data storage
        zstr_long l;     // Heap mode: ptr, len, cap
        zstr_short s;    // SSO mode: 23-byte buffer + length
    };
};
// Total: 32 bytes (exactly half a cache line on most systems)
```

### Design Benefits

1. **Compact Size**: 32 bytes fits exactly in half a 64-byte cache line
2. **Fast Discriminator**: `is_long` flag checked first for optimal branch prediction
3. **Aligned Union**: 8-byte alignment ensures efficient memory access
4. **Small String Optimization**: Strings ≤22 chars never touch the heap

### Cache Locality Best Practices

```c
// ✓ GOOD: Sequential access (cache-friendly)
zstr strings[1000];
for (int i = 0; i < 1000; i++) {
    process(zstr_len(&strings[i]));
}

// ✗ BAD: Random access (cache-unfriendly)
for (int i = 0; i < 1000; i++) {
    int idx = random() % 1000;
    process(zstr_len(&strings[idx]));
}
```

## 3. Optimized File I/O

The `zstr_read_file()` function uses chunked reading for better performance:

```c
// Reads in 64KB chunks for optimal cache/disk interaction
zstr content = zstr_read_file("large_file.txt");
```

### Implementation Details

- **Chunk Size**: 64KB (typical filesystem block size)
- **Single Allocation**: Pre-allocates exact needed capacity
- **Cache-Friendly**: Aligned chunks reduce cache misses
- **Error Handling**: Robust handling of partial reads

### File I/O Performance

Our benchmarks show excellent file reading performance:

- **Standard**: 11.5 GB/s throughput
- **With mimalloc**: 12.8 GB/s throughput

## 4. Enhanced Benchmarking

Run comprehensive benchmarks to measure performance:

```bash
# Run all benchmarks
make bench

# Run specific benchmarks
make bench_c              # Basic zstr vs malloc comparison
make bench_optimized      # Comprehensive optimization tests
make bench_sds            # Comparison with SDS library

# With mimalloc
make USE_MIMALLOC=1 bench_optimized
```

### Benchmark Categories

1. **Memory Allocation Patterns**
   - SSO (Small String Optimization) performance
   - Heap growth patterns
   - Pre-allocated vs dynamic allocation

2. **File I/O**
   - Large file reading (1MB × 100 iterations)
   - Throughput measurement (MB/s)

3. **String Operations**
   - Small string operations (SSO)
   - Large string operations (heap)
   - Operation timing (ns/µs per operation)

4. **Cache Locality**
   - Sequential access patterns
   - Strided access patterns
   - Cache efficiency comparison

## Performance Tips

### 1. Use SSO When Possible
```c
// ✓ Fits in SSO (≤22 chars): no heap allocation
zstr s = zstr_lit("Short string");

// ✗ Requires heap (>22 chars)
zstr s = zstr_from("This is a longer string that needs heap allocation");
```

### 2. Pre-allocate for Known Sizes
```c
// ✓ Pre-allocate if you know the final size
zstr s = zstr_with_capacity(10000);
for (int i = 0; i < 1000; i++) {
    zstr_cat(&s, "append");
}

// ✗ Multiple reallocations (slower)
zstr s = zstr_init();
for (int i = 0; i < 1000; i++) {
    zstr_cat(&s, "append");
}
```

### 3. Reuse Buffers
```c
// ✓ Reuse the same buffer
zstr buf = zstr_with_capacity(1000);
for (int i = 0; i < 100; i++) {
    zstr_clear(&buf);
    zstr_fmt(&buf, "Item %d", i);
    process(&buf);
}
zstr_free(&buf);

// ✗ Repeated allocation/deallocation
for (int i = 0; i < 100; i++) {
    zstr s = zstr_init();
    zstr_fmt(&s, "Item %d", i);
    process(&s);
    zstr_free(&s);
}
```

### 4. Enable mimalloc in Production
```bash
# Compile with mimalloc for production builds
gcc -O3 -DUSE_MIMALLOC -march=native myapp.c -lmimalloc
```

## Compiler Optimizations

For maximum performance, use these compiler flags:

```bash
# Recommended flags
gcc -O3 -march=native -Wall -Wextra mycode.c

# With Link-Time Optimization (LTO)
gcc -O3 -march=native -flto -Wall -Wextra mycode.c
```

## Memory Usage

The optimizations maintain the same memory footprint:

- **zstr struct**: 32 bytes (unchanged)
- **SSO capacity**: 23 bytes (22 chars + null terminator)
- **Heap overhead**: mimalloc has ~16 bytes per allocation vs ~24 for glibc

## Conclusion

These optimizations provide significant performance improvements while maintaining full API compatibility:

- **10-25% faster** allocations with mimalloc
- **Better cache utilization** with aligned data structures
- **Optimized file I/O** with chunked reading
- **Comprehensive benchmarks** to measure real-world impact

Enable mimalloc and use the optimization best practices for maximum performance in your applications.
