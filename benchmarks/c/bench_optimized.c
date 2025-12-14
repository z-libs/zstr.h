// Enhanced benchmark to test optimizations:
// - File reading performance
// - Cache locality improvements
// - mimalloc integration

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "zstr.h"

#define ITER_COUNT 1000000

static double now() 
{
#if defined(__APPLE__) || !defined(CLOCK_MONOTONIC)
    // Fallback for systems without clock_gettime (e.g., older macOS)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

// Benchmark: Memory allocation patterns (cache locality)
void bench_allocation_patterns()
{
    printf("\n=== Memory Allocation Pattern Benchmark ===\n");
    
    // Test 1: Sequential small allocations (SSO)
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
    
    // Test 2: Heap allocations with growth
    start = now();
    zstr s = zstr_init();
    for (int i = 0; i < 10000; i++)
    {
        zstr_cat(&s, "Growing string to test heap allocation patterns. ");
    }
    elapsed = now() - start;
    printf("[Heap Growth]     10000 appends: %.4fs (final size: %zu)\n", 
           elapsed, zstr_len(&s));
    zstr_free(&s);
    
    // Test 3: Pre-allocated capacity
    start = now();
    zstr s2 = zstr_with_capacity(500000);
    for (int i = 0; i < 10000; i++)
    {
        zstr_cat(&s2, "Pre-allocated buffer avoids reallocation overhead. ");
    }
    elapsed = now() - start;
    printf("[Pre-allocated]   10000 appends: %.4fs (final size: %zu)\n", 
           elapsed, zstr_len(&s2));
    zstr_free(&s2);
}

// Benchmark: File I/O performance
void bench_file_io()
{
    printf("\n=== File I/O Benchmark ===\n");
    
    // Create a test file in a portable way
    // Try to use system temp directory
    const char *temp_dir = getenv("TMPDIR");
    if (!temp_dir) temp_dir = getenv("TEMP");
    if (!temp_dir) temp_dir = getenv("TMP");
#ifdef _WIN32
    if (!temp_dir) temp_dir = "C:\\Windows\\Temp";
#else
    if (!temp_dir) temp_dir = "/tmp";
#endif
    
    char test_file[512];
    snprintf(test_file, sizeof(test_file), "%s/zstr_test.txt", temp_dir);
    
    FILE *f = fopen(test_file, "w");
    if (!f) {
        printf("Error: Could not create test file at %s\n", test_file);
        return;
    }
    
    // Write 1MB of test data
    const char *pattern = "This is a test line for benchmarking file reading performance.\n";
    size_t pattern_len = strlen(pattern);
    size_t target_size = 1024 * 1024; // 1MB
    size_t written = 0;
    
    while (written < target_size)
    {
        fwrite(pattern, 1, pattern_len, f);
        written += pattern_len;
    }
    fclose(f);
    
    // Benchmark: Read the file multiple times
    double start = now();
    int iterations = 100;
    size_t total_bytes = 0;
    
    for (int i = 0; i < iterations; i++)
    {
        zstr content = zstr_read_file(test_file);
        total_bytes += zstr_len(&content);
        zstr_free(&content);
    }
    
    double elapsed = now() - start;
    double mb_per_sec = (total_bytes / (1024.0 * 1024.0)) / elapsed;
    
    printf("[File Read]       %d iterations of 1MB: %.4fs (%.2f MB/s)\n", 
           iterations, elapsed, mb_per_sec);
    
    // Cleanup
    remove(test_file);
}

// Benchmark: String operations with various sizes
void bench_string_operations()
{
    printf("\n=== String Operations Benchmark ===\n");
    
    // Test 1: Small string operations (SSO)
    double start = now();
    int iterations = 100000;
    
    for (int i = 0; i < iterations; i++)
    {
        zstr s = zstr_from("hello");
        zstr_cat(&s, " world");
        zstr_to_upper(&s);
        volatile size_t len = zstr_len(&s);
        (void)len;
        zstr_free(&s);
    }
    double elapsed = now() - start;
    printf("[Small Ops]       %d iterations: %.4fs (%.2f ns/op)\n", 
           iterations, elapsed, (elapsed * 1e9) / iterations);
    
    // Test 2: Large string operations
    start = now();
    iterations = 1000;
    
    for (int i = 0; i < iterations; i++)
    {
        zstr s = zstr_from("This is a longer string that will be allocated on the heap");
        zstr_cat(&s, " and we'll append more data to it multiple times");
        zstr_cat(&s, " to test performance with larger allocations");
        zstr_replace(&s, "string", "text");
        volatile size_t len = zstr_len(&s);
        (void)len;
        zstr_free(&s);
    }
    elapsed = now() - start;
    printf("[Large Ops]       %d iterations: %.4fs (%.2f µs/op)\n", 
           iterations, elapsed, (elapsed * 1e6) / iterations);
}

// Benchmark: Cache-friendly vs cache-unfriendly access patterns
void bench_cache_locality()
{
    printf("\n=== Cache Locality Benchmark ===\n");
    
    const int num_strings = 1000;
    zstr *strings = malloc(num_strings * sizeof(zstr));
    
    // Initialize strings
    for (int i = 0; i < num_strings; i++)
    {
        strings[i] = zstr_from("Testing cache locality with sequential access patterns");
    }
    
    // Test 1: Sequential access (cache-friendly)
    double start = now();
    volatile size_t sum = 0;
    for (int iter = 0; iter < 10000; iter++)
    {
        for (int i = 0; i < num_strings; i++)
        {
            sum += zstr_len(&strings[i]);
        }
    }
    double elapsed = now() - start;
    printf("[Sequential]      10000 iterations: %.4fs\n", elapsed);
    
    // Test 2: Strided access (less cache-friendly)
    start = now();
    sum = 0;
    for (int iter = 0; iter < 10000; iter++)
    {
        for (int i = 0; i < num_strings; i += 8)
        {
            sum += zstr_len(&strings[i]);
        }
    }
    elapsed = now() - start;
    printf("[Strided (8)]     10000 iterations: %.4fs\n", elapsed);
    
    // Cleanup
    for (int i = 0; i < num_strings; i++)
    {
        zstr_free(&strings[i]);
    }
    free(strings);
}

int main(void)
{
    printf("╔════════════════════════════════════════════════════════════════════╗\n");
    printf("║          zstr.h - Optimization Benchmark Suite                    ║\n");
    printf("╚════════════════════════════════════════════════════════════════════╝\n");
    
#ifdef USE_MIMALLOC
    printf("\n✓ Using mimalloc allocator\n");
#else
    printf("\n✗ Using standard allocator (set USE_MIMALLOC=1 for better performance)\n");
#endif
    
    printf("\nSystem info:\n");
    printf("  zstr size: %zu bytes\n", sizeof(zstr));
    printf("  SSO capacity: %d bytes\n", ZSTR_SSO_CAP);
    
    bench_allocation_patterns();
    bench_file_io();
    bench_string_operations();
    bench_cache_locality();
    
    printf("\n✓ All benchmarks completed successfully\n");
    return 0;
}
