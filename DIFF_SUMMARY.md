# Unified Diff vs Parent Repository

This document provides a summary of the differences between this fork (`EdgeOfAssembly/zstr.h`) and the parent repository (`z-libs/zstr.h`).

## Files

- **`unified-diff-vs-parent.patch`**: The complete unified diff showing all changes between this fork and the parent repository

## How to Apply the Diff

To view the differences:
```bash
less unified-diff-vs-parent.patch
```

To apply these changes to a copy of the parent repository:
```bash
cd /path/to/z-libs/zstr.h
patch -p1 < /path/to/unified-diff-vs-parent.patch
```

## Summary of Changes

### Statistics
- **Files changed**: 9 files
- **Lines added**: 1112 additions
- **Lines removed**: 68 deletions
- **Diff size**: ~46 KB

### Modified Files

1. **`.gitignore`** - Added benchmark binary to ignore list
2. **`BENCHMARK_RESULTS.md`** (new) - Comprehensive benchmark results documentation
3. **`Makefile`** - Extensive enhancements for build system
4. **`OPTIMIZATIONS.md`** (new) - Detailed optimization documentation
5. **`PR_SUMMARY.md`** (new) - Pull request summary and changes
6. **`_codeql_detected_source_root`** (new) - CodeQL configuration
7. **`benchmarks/c/bench_optimized.c`** (new) - New optimized benchmark suite
8. **`src/zstr.c`** - Core implementation improvements
9. **`zstr.h`** - Header file with optimizations

### Key Features Added

This fork includes several enhancements over the parent repository:

1. **Performance Optimizations**
   - Small String Optimization (SSO) improvements
   - Memory allocation optimizations with mimalloc support
   - Cache-friendly data structures
   - Efficient chunked file reading (64KB blocks)

2. **Enhanced Build System**
   - Advanced Makefile with multiple build targets
   - mimalloc integration support
   - Benchmark compilation targets

3. **Comprehensive Documentation**
   - Detailed benchmark results
   - Optimization strategies and rationale
   - Performance comparison data

4. **Testing & Benchmarking**
   - New optimized benchmark suite
   - Performance regression testing
   - Memory allocation pattern tests

## Generating This Diff

This diff was generated using:
```bash
git remote add upstream https://github.com/z-libs/zstr.h
git fetch upstream
git diff --unified=3 upstream/main HEAD > unified-diff-vs-parent.patch
```

## Date Generated

- **Generated**: 2025-12-02
- **Upstream Reference**: z-libs/zstr.h @ main branch
- **Fork Reference**: EdgeOfAssembly/zstr.h @ copilot/generate-unified-diff branch
