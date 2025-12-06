// Advanced optimization benchmark for zstr.h
// Tests: Prefetch, SIMD, OpenMP, and combined optimizations

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "zstr.h"

#define ITER_COUNT 1000000
#define LARGE_ITER 10000
#define SMALL_ITER 100000

static double now() 
{
#if defined(__APPLE__) || !defined(CLOCK_MONOTONIC)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

// Baseline tests (same as bench_optimized.c)
void bench_baseline()
{
    printf("\n=== Baseline Performance (Standard) ===\n");
    
    // Test 1: SSO Sequential
    double start = now();
    volatile size_t sum = 0;
    for (int i = 0; i < ITER_COUNT; i++)
    {
        zstr s = zstr_from("Test string for SSO");
        sum += zstr_len(&s);
        zstr_free(&s);
    }
    double elapsed = now() - start;
    printf("[SSO Sequential]  %d iterations: %.4fs (%.2f ns/op)\n", 
           ITER_COUNT, elapsed, (elapsed * 1e9) / ITER_COUNT);
    
    // Test 2: Small string operations
    start = now();
    for (int i = 0; i < SMALL_ITER; i++)
    {
        zstr s = zstr_from("hello");
        zstr_cat(&s, " world");
        zstr_to_upper(&s);
        volatile size_t len = zstr_len(&s);
        (void)len;
        zstr_free(&s);
    }
    elapsed = now() - start;
    printf("[Small Ops]       %d iterations: %.4fs (%.2f ns/op)\n", 
           SMALL_ITER, elapsed, (elapsed * 1e9) / SMALL_ITER);
    
    // Test 3: Large concat operations
    start = now();
    zstr s = zstr_init();
    for (int i = 0; i < LARGE_ITER; i++)
    {
        zstr_cat(&s, "Growing string to test heap allocation patterns. ");
    }
    elapsed = now() - start;
    printf("[Heap Growth]     %d appends: %.4fs (final size: %zu)\n", 
           LARGE_ITER, elapsed, zstr_len(&s));
    zstr_free(&s);
}

// Test prefetch optimization
void bench_prefetch()
{
    printf("\n=== Prefetch Optimization Test ===\n");
    
    const int num_strings = 1000;
    zstr *strings = malloc(num_strings * sizeof(zstr));
    
    // Initialize strings
    for (int i = 0; i < num_strings; i++)
    {
        strings[i] = zstr_from("Testing prefetch with sequential access patterns for better performance");
    }
    
    // Test without prefetch (baseline)
    double start = now();
    volatile size_t sum = 0;
    for (int iter = 0; iter < LARGE_ITER; iter++)
    {
        for (int i = 0; i < num_strings; i++)
        {
            sum += zstr_len(&strings[i]);
        }
    }
    double elapsed = now() - start;
    printf("[No Prefetch]     %d iterations: %.4fs\n", LARGE_ITER, elapsed);
    
    // Test with prefetch
    start = now();
    sum = 0;
    for (int iter = 0; iter < LARGE_ITER; iter++)
    {
        for (int i = 0; i < num_strings; i++)
        {
            // Prefetch next item
#if defined(__GNUC__) || defined(__clang__)
            if (i + 1 < num_strings) {
                __builtin_prefetch(&strings[i + 1], 0, 3);
            }
#endif
            sum += zstr_len(&strings[i]);
        }
    }
    elapsed = now() - start;
    printf("[With Prefetch]   %d iterations: %.4fs\n", LARGE_ITER, elapsed);
    
    // Cleanup
    for (int i = 0; i < num_strings; i++)
    {
        zstr_free(&strings[i]);
    }
    free(strings);
}

// Test SIMD operations (character processing)
void bench_simd()
{
    printf("\n=== SIMD Optimization Test ===\n");
    
    // Create a large string for processing
    zstr s = zstr_init();
    const char *pattern = "The quick brown fox jumps over the lazy dog. ";
    for (int i = 0; i < 1000; i++) {
        zstr_cat(&s, pattern);
    }
    
    // Test uppercase conversion - baseline
    double start = now();
    for (int i = 0; i < 100; i++) {
        zstr copy = zstr_dup(&s);
        zstr_to_upper(&copy);
        zstr_free(&copy);
    }
    double elapsed = now() - start;
    printf("[Uppercase Base]  100 iterations: %.4fs (%.2f µs/op)\n", 
           elapsed, (elapsed * 1e6) / 100);
    
    // Test character counting - baseline
    start = now();
    volatile size_t count = 0;
    for (int i = 0; i < 1000; i++) {
        const char *p = zstr_cstr(&s);
        size_t len = zstr_len(&s);
        size_t c = 0;
        for (size_t j = 0; j < len; j++) {
            if (p[j] == ' ') c++;
        }
        count += c;
    }
    elapsed = now() - start;
    printf("[Char Count Base] 1000 iterations: %.4fs (%.2f µs/op)\n", 
           elapsed, (elapsed * 1e6) / 1000);
    
    zstr_free(&s);
}

// Test OpenMP parallelization
void bench_parallel()
{
    printf("\n=== Parallel Processing Test ===\n");
    
#ifdef _OPENMP
    printf("OpenMP enabled: %d threads available\n", omp_get_max_threads());
#else
    printf("OpenMP not available - sequential processing only\n");
#endif
    
    const int num_strings = 1000;
    zstr *strings = malloc(num_strings * sizeof(zstr));
    
    // Initialize strings
    for (int i = 0; i < num_strings; i++) {
        strings[i] = zstr_from("Testing parallel string processing with OpenMP for better performance on multi-core systems");
    }
    
    // Sequential processing
    double start = now();
    for (int i = 0; i < num_strings; i++) {
        zstr copy = zstr_dup(&strings[i]);
        zstr_to_upper(&copy);
        zstr_free(&copy);
    }
    double elapsed = now() - start;
    printf("[Sequential]      %d ops: %.4fs\n", num_strings, elapsed);
    
#ifdef _OPENMP
    // Parallel processing
    start = now();
    #pragma omp parallel for
    for (int i = 0; i < num_strings; i++) {
        zstr copy = zstr_dup(&strings[i]);
        zstr_to_upper(&copy);
        zstr_free(&copy);
    }
    elapsed = now() - start;
    printf("[Parallel]        %d ops: %.4fs (%.2fx speedup)\n", 
           num_strings, elapsed, 
           (elapsed > 0) ? ((start - (now() - elapsed)) / elapsed) : 0.0);
#endif
    
    // Cleanup
    for (int i = 0; i < num_strings; i++) {
        zstr_free(&strings[i]);
    }
    free(strings);
}

// Test combined optimizations
void bench_combined()
{
    printf("\n=== Combined Optimizations Test ===\n");
    
    const int num_strings = 10000;
    zstr *strings = malloc(num_strings * sizeof(zstr));
    
    // Initialize
    for (int i = 0; i < num_strings; i++) {
        char buf[100];
        snprintf(buf, sizeof(buf), "String #%d with some content", i);
        strings[i] = zstr_from(buf);
    }
    
    // Test: Bulk operations with all optimizations
    double start = now();
    volatile size_t total_len = 0;
    
    // With prefetch
    for (int i = 0; i < num_strings; i++) {
#if defined(__GNUC__) || defined(__clang__)
        if (i + 1 < num_strings) {
            __builtin_prefetch(&strings[i + 1], 0, 3);
        }
#endif
        total_len += zstr_len(&strings[i]);
    }
    
    double elapsed = now() - start;
    printf("[Bulk Access]     %d strings: %.4fs (%.2f ns/op)\n", 
           num_strings, elapsed, (elapsed * 1e9) / num_strings);
    
    // Cleanup
    for (int i = 0; i < num_strings; i++) {
        zstr_free(&strings[i]);
    }
    free(strings);
}

int main(void)
{
    printf("╔════════════════════════════════════════════════════════════════════╗\n");
    printf("║          zstr.h - Advanced Optimization Suite                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════════╝\n");
    
#ifdef USE_MIMALLOC
    printf("\n✓ Using mimalloc allocator\n");
#else
    printf("\n✗ Using standard allocator\n");
#endif

#ifdef __AVX2__
    printf("✓ AVX2 SIMD available\n");
#elif defined(__SSE4_2__)
    printf("✓ SSE4.2 SIMD available\n");
#elif defined(__SSE2__)
    printf("✓ SSE2 SIMD available\n");
#else
    printf("✗ No SIMD extensions detected\n");
#endif

#ifdef _OPENMP
    printf("✓ OpenMP available\n");
#else
    printf("✗ OpenMP not enabled\n");
#endif
    
    printf("\nSystem info:\n");
    printf("  zstr size: %zu bytes\n", sizeof(zstr));
    printf("  SSO capacity: %d bytes\n", ZSTR_SSO_CAP);
    
    bench_baseline();
    bench_prefetch();
    bench_simd();
    bench_parallel();
    bench_combined();
    
    printf("\n✓ All benchmarks completed successfully\n");
    return 0;
}
