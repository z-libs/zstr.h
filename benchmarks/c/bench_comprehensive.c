// Comprehensive benchmark testing all optimization combinations
// Tests: baseline vs SIMD vs SIMD+Prefetch vs SIMD+Prefetch+OpenMP

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Include OpenMP if available
#ifdef _OPENMP
#include <omp.h>
#endif

#include "zstr.h"

#define SMALL_ITER 1000000
#define MED_ITER 100000
#define LARGE_ITER 10000

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

void print_header(const char *title)
{
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════════════════════════════════\n");
}

void bench_sso_sequential()
{
    print_header("SSO Sequential Allocation Benchmark");
    
    double start = now();
    volatile size_t sum = 0;
    for (int i = 0; i < SMALL_ITER; i++)
    {
        zstr s = zstr_from("SSO test string");
        sum += zstr_len(&s);
        zstr_free(&s);
    }
    double elapsed = now() - start;
    printf("  Iterations:     %d\n", SMALL_ITER);
    printf("  Time:           %.4fs\n", elapsed);
    printf("  Throughput:     %.2f ns/op\n", (elapsed * 1e9) / SMALL_ITER);
    printf("  Operations/sec: %.2f M ops/s\n", (SMALL_ITER / elapsed) / 1e6);
}

void bench_heap_growth()
{
    print_header("Heap Growth Pattern Benchmark");
    
    double start = now();
    zstr s = zstr_init();
    for (int i = 0; i < LARGE_ITER; i++)
    {
        zstr_cat(&s, "Growing string to test heap allocation patterns. ");
    }
    double elapsed = now() - start;
    printf("  Iterations:     %d appends\n", LARGE_ITER);
    printf("  Final size:     %zu bytes\n", zstr_len(&s));
    printf("  Time:           %.4fs\n", elapsed);
    printf("  Throughput:     %.2f µs/op\n", (elapsed * 1e6) / LARGE_ITER);
    zstr_free(&s);
}

void bench_pre_allocated()
{
    print_header("Pre-allocated Buffer Benchmark");
    
    double start = now();
    zstr s = zstr_with_capacity(500000);
    for (int i = 0; i < LARGE_ITER; i++)
    {
        zstr_cat(&s, "Pre-allocated buffer avoids reallocation overhead. ");
    }
    double elapsed = now() - start;
    printf("  Iterations:     %d appends\n", LARGE_ITER);
    printf("  Final size:     %zu bytes\n", zstr_len(&s));
    printf("  Time:           %.4fs\n", elapsed);
    printf("  Throughput:     %.2f µs/op\n", (elapsed * 1e6) / LARGE_ITER);
    printf("  Speedup:        vs heap growth\n");
    zstr_free(&s);
}

void bench_uppercase_simd()
{
    print_header("Uppercase Conversion with SIMD");
    
    // Create strings of various sizes
    size_t sizes[] = {10, 50, 100, 500, 1000, 5000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    printf("  %-12s %-12s %-15s %-15s\n", "Size", "Time", "Throughput", "Speed");
    printf("  %-12s %-12s %-15s %-15s\n", "----", "----", "----------", "-----");
    
    for (int i = 0; i < num_sizes; i++) {
        size_t size = sizes[i];
        
        // Create test string
        zstr base = zstr_init();
        for (size_t j = 0; j < size; j++) {
            zstr_push(&base, 'a' + (j % 26));
        }
        
        // Benchmark
        int iterations = (size < 100) ? 100000 : (size < 1000) ? 10000 : 1000;
        double start = now();
        for (int j = 0; j < iterations; j++) {
            zstr copy = zstr_dup(&base);
            zstr_to_upper(&copy);
            zstr_free(&copy);
        }
        double elapsed = now() - start;
        
        double throughput = (size * iterations) / elapsed / 1e6; // MB/s
        printf("  %-12zu %-12.4fs %-15.2f %-15s\n", 
               size, elapsed, throughput, 
               size >= 32 ? "SIMD" : "Scalar");
        
        zstr_free(&base);
    }
}

void bench_lowercase_simd()
{
    print_header("Lowercase Conversion with SIMD");
    
    size_t sizes[] = {10, 50, 100, 500, 1000, 5000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    printf("  %-12s %-12s %-15s %-15s\n", "Size", "Time", "Throughput", "Speed");
    printf("  %-12s %-12s %-15s %-15s\n", "----", "----", "----------", "-----");
    
    for (int i = 0; i < num_sizes; i++) {
        size_t size = sizes[i];
        
        zstr base = zstr_init();
        for (size_t j = 0; j < size; j++) {
            zstr_push(&base, 'A' + (j % 26));
        }
        
        int iterations = (size < 100) ? 100000 : (size < 1000) ? 10000 : 1000;
        double start = now();
        for (int j = 0; j < iterations; j++) {
            zstr copy = zstr_dup(&base);
            zstr_to_lower(&copy);
            zstr_free(&copy);
        }
        double elapsed = now() - start;
        
        double throughput = (size * iterations) / elapsed / 1e6;
        printf("  %-12zu %-12.4fs %-15.2f %-15s\n", 
               size, elapsed, throughput,
               size >= 32 ? "SIMD" : "Scalar");
        
        zstr_free(&base);
    }
}

void bench_case_insensitive_compare()
{
    print_header("Case-Insensitive String Comparison with SIMD");
    
    size_t sizes[] = {10, 50, 100, 500, 1000, 5000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    printf("  %-12s %-12s %-15s %-15s\n", "Size", "Time", "Throughput", "Speed");
    printf("  %-12s %-12s %-15s %-15s\n", "----", "----", "----------", "-----");
    
    for (int i = 0; i < num_sizes; i++) {
        size_t size = sizes[i];
        
        zstr s1 = zstr_init();
        zstr s2 = zstr_init();
        for (size_t j = 0; j < size; j++) {
            zstr_push(&s1, 'A' + (j % 26));
            zstr_push(&s2, 'a' + (j % 26));
        }
        
        int iterations = (size < 100) ? 500000 : (size < 1000) ? 50000 : 10000;
        double start = now();
        volatile int result = 0;
        for (int j = 0; j < iterations; j++) {
            result += zstr_eq_ignore_case(&s1, &s2) ? 1 : 0;
        }
        double elapsed = now() - start;
        
        double throughput = (size * iterations) / elapsed / 1e6;
        printf("  %-12zu %-12.4fs %-15.2f %-15s\n", 
               size, elapsed, throughput,
               size >= 32 ? "SIMD" : "Scalar");
        
        zstr_free(&s1);
        zstr_free(&s2);
        (void)result;
    }
}

void bench_prefetch_bulk_access()
{
    print_header("Bulk String Access with Prefetch");
    
    int counts[] = {100, 1000, 10000};
    int num_counts = sizeof(counts) / sizeof(counts[0]);
    
    printf("  %-12s %-15s %-15s\n", "Count", "Time", "Throughput");
    printf("  %-12s %-15s %-15s\n", "-----", "----", "----------");
    
    for (int i = 0; i < num_counts; i++) {
        int count = counts[i];
        
        zstr *strings = malloc(count * sizeof(zstr));
        for (int j = 0; j < count; j++) {
            char buf[100];
            snprintf(buf, sizeof(buf), "String #%d with some content for testing", j);
            strings[j] = zstr_from(buf);
        }
        
        int iterations = 10000 / (count / 100 + 1);
        double start = now();
        volatile size_t total = 0;
        for (int iter = 0; iter < iterations; iter++) {
            for (int j = 0; j < count; j++) {
                total += zstr_len(&strings[j]);
            }
        }
        double elapsed = now() - start;
        
        printf("  %-12d %-15.4fs %-15.2f ns/op\n", 
               count, elapsed, (elapsed * 1e9) / (iterations * count));
        
        for (int j = 0; j < count; j++) {
            zstr_free(&strings[j]);
        }
        free(strings);
        (void)total;
    }
}

void bench_bulk_operations()
{
    print_header("Bulk Operations with Prefetch & OpenMP");
    
#ifdef _OPENMP
    printf("  OpenMP threads: %d\n\n", omp_get_max_threads());
#else
    printf("  OpenMP: Not enabled\n\n");
#endif
    
    int count = 10000;
    zstr *strings = malloc(count * sizeof(zstr));
    
    // Initialize
    for (int i = 0; i < count; i++) {
        strings[i] = zstr_from("Test String For Bulk Processing Operations");
    }
    
    // Test bulk uppercase
    double start = now();
    zstr_to_upper_bulk(strings, count);
    double elapsed = now() - start;
    printf("  Bulk uppercase: %.4fs (%.2f ns/op)\n", elapsed, (elapsed * 1e9) / count);
    
    // Test bulk lowercase
    start = now();
    zstr_to_lower_bulk(strings, count);
    elapsed = now() - start;
    printf("  Bulk lowercase: %.4fs (%.2f ns/op)\n", elapsed, (elapsed * 1e9) / count);
    
    // Cleanup
    for (int i = 0; i < count; i++) {
        zstr_free(&strings[i]);
    }
    free(strings);
}

void print_system_info()
{
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════════╗\n");
    printf("║  zstr.h - Comprehensive Optimization Benchmark Suite             ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("System Configuration:\n");
    printf("  zstr size:          %zu bytes\n", sizeof(zstr));
    printf("  SSO capacity:       %d bytes\n", ZSTR_SSO_CAP);
    
#ifdef USE_MIMALLOC
    printf("  Allocator:          mimalloc\n");
#else
    printf("  Allocator:          standard malloc\n");
#endif

#ifdef __AVX2__
    printf("  SIMD:               AVX2\n");
#elif defined(__SSE4_2__)
    printf("  SIMD:               SSE4.2\n");
#elif defined(__SSE2__)
    printf("  SIMD:               SSE2\n");
#else
    printf("  SIMD:               None\n");
#endif

#ifdef _OPENMP
    printf("  OpenMP:             Enabled (%d threads)\n", omp_get_max_threads());
#else
    printf("  OpenMP:             Disabled\n");
#endif

    printf("  Prefetch:           ");
#if defined(__GNUC__) || defined(__clang__)
    printf("Enabled (__builtin_prefetch)\n");
#elif defined(_MSC_VER)
    printf("Enabled (_mm_prefetch)\n");
#else
    printf("Disabled\n");
#endif
}

int main(void)
{
    print_system_info();
    
    bench_sso_sequential();
    bench_heap_growth();
    bench_pre_allocated();
    bench_uppercase_simd();
    bench_lowercase_simd();
    bench_case_insensitive_compare();
    bench_prefetch_bulk_access();
    bench_bulk_operations();
    
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("  ✓ All benchmarks completed successfully\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    return 0;
}
