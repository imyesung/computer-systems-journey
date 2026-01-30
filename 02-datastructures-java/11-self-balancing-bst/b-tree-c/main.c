#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "b-tree.h"

/* ================================================================
 * TEST UTILITIES
 * ================================================================ */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("\n[TEST] %s\n", name)
#define ASSERT(cond, msg) do { \
    if (cond) { \
        tests_passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        tests_failed++; \
        printf("  FAIL: %s\n", msg); \
    } \
} while(0)

/* Shuffle array using Fisher-Yates algorithm */
static void shuffle(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

/* ================================================================
 * BASIC FUNCTIONALITY TESTS
 * ================================================================ */

static void test_create_destroy(void) {
    TEST("Create and Destroy");

    BTree *tree = btree_create(3);
    ASSERT(tree != NULL, "btree_create returns non-NULL");
    ASSERT(tree->t == 3, "minimum degree is set correctly");
    ASSERT(tree->root != NULL, "root is allocated");
    ASSERT(tree->root->n == 0, "root starts empty");
    ASSERT(tree->root->is_leaf == true, "root starts as leaf");
    ASSERT(btree_validate(tree), "empty tree is valid");

    btree_destroy(tree);
    printf("  PASS: btree_destroy completed\n");
    tests_passed++;
}

static void test_insert_search(void) {
    TEST("Insert and Search");

    BTree *tree = btree_create(3);
    int keys[] = {10, 20, 5, 6, 12, 30, 7, 17};
    int n = sizeof(keys) / sizeof(keys[0]);

    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }

    ASSERT(btree_count(tree) == n, "count matches inserted keys");
    ASSERT(btree_validate(tree), "tree valid after inserts");

    /* Search for existing keys */
    int idx;
    for (int i = 0; i < n; i++) {
        BTreeNode *found = btree_search(tree->root, keys[i], &idx);
        ASSERT(found != NULL && found->keys[idx] == keys[i],
               "search finds inserted key");
    }

    /* Search for non-existing keys */
    ASSERT(btree_search(tree->root, 100, &idx) == NULL,
           "search returns NULL for missing key");
    ASSERT(btree_search(tree->root, 0, &idx) == NULL,
           "search returns NULL for missing key");

    btree_destroy(tree);
}

/* ================================================================
 * DELETE TESTS
 * ================================================================ */

static void test_delete_from_leaf(void) {
    TEST("Delete from Leaf (Case 1)");

    BTree *tree = btree_create(3);
    /* Insert keys that stay in a single leaf node */
    int keys[] = {1, 2, 3, 4, 5};
    int n = sizeof(keys) / sizeof(keys[0]);

    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }

    ASSERT(btree_count(tree) == n, "initial count correct");

    /* Delete from leaf */
    btree_delete(tree, 3);
    ASSERT(btree_count(tree) == n - 1, "count decreases after delete");
    ASSERT(btree_search(tree->root, 3, NULL) == NULL, "deleted key not found");
    ASSERT(btree_validate(tree), "tree valid after delete");

    /* Delete more keys */
    btree_delete(tree, 1);
    btree_delete(tree, 5);
    ASSERT(btree_count(tree) == n - 3, "count correct after multiple deletes");
    ASSERT(btree_validate(tree), "tree still valid");

    btree_destroy(tree);
}

static void test_delete_with_predecessor(void) {
    TEST("Delete from Internal Node (Case 2a - Predecessor)");

    BTree *tree = btree_create(2);  /* 2-3-4 tree for simpler structure */

    /* Build a tree where internal node deletion uses predecessor */
    int keys[] = {10, 20, 30, 5, 15, 25, 35, 3, 7};
    int n = sizeof(keys) / sizeof(keys[0]);

    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }

    int count_before = btree_count(tree);
    ASSERT(btree_validate(tree), "tree valid before delete");

    /* Delete a key that's in an internal node */
    btree_delete(tree, 10);
    ASSERT(btree_count(tree) == count_before - 1, "count decreases");
    ASSERT(btree_search(tree->root, 10, NULL) == NULL, "deleted key not found");
    ASSERT(btree_validate(tree), "tree valid after predecessor replacement");

    btree_destroy(tree);
}

static void test_delete_with_successor(void) {
    TEST("Delete from Internal Node (Case 2b - Successor)");

    BTree *tree = btree_create(2);

    /* Insert keys in specific order to test successor case */
    int keys[] = {20, 10, 30, 5, 25, 35, 27, 33, 37};
    int n = sizeof(keys) / sizeof(keys[0]);

    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }

    int count_before = btree_count(tree);
    ASSERT(btree_validate(tree), "tree valid before delete");

    btree_delete(tree, 30);
    ASSERT(btree_count(tree) == count_before - 1, "count decreases");
    ASSERT(btree_search(tree->root, 30, NULL) == NULL, "deleted key not found");
    ASSERT(btree_validate(tree), "tree valid after successor replacement");

    btree_destroy(tree);
}

static void test_delete_with_merge(void) {
    TEST("Delete with Merge (Case 2c and 3c)");

    BTree *tree = btree_create(2);

    /* Insert enough keys to create multiple levels */
    for (int i = 1; i <= 10; i++) {
        btree_insert(tree, i);
    }

    int count_before = btree_count(tree);
    int height_before = btree_height(tree);
    ASSERT(btree_validate(tree), "tree valid before deletes");

    /* Delete keys to trigger merges */
    for (int i = 1; i <= 5; i++) {
        btree_delete(tree, i);
        ASSERT(btree_validate(tree), "tree valid during deletions");
    }

    ASSERT(btree_count(tree) == count_before - 5, "count correct after merges");
    ASSERT(btree_height(tree) <= height_before, "height may decrease");

    btree_destroy(tree);
}

static void test_delete_with_borrow(void) {
    TEST("Delete with Borrow (Case 3a and 3b)");

    BTree *tree = btree_create(3);

    /* Insert keys to create a structure where borrowing is needed */
    for (int i = 1; i <= 20; i++) {
        btree_insert(tree, i);
    }

    ASSERT(btree_validate(tree), "tree valid before deletes");

    /* Delete keys that trigger borrowing from siblings */
    btree_delete(tree, 1);
    ASSERT(btree_validate(tree), "valid after delete 1");

    btree_delete(tree, 20);
    ASSERT(btree_validate(tree), "valid after delete 20");

    btree_delete(tree, 10);
    ASSERT(btree_validate(tree), "valid after delete 10");

    btree_destroy(tree);
}

static void test_delete_all_keys(void) {
    TEST("Delete All Keys");

    BTree *tree = btree_create(3);

    int keys[] = {50, 25, 75, 10, 30, 60, 90, 5, 15, 27, 35, 55, 65, 80, 95};
    int n = sizeof(keys) / sizeof(keys[0]);

    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }

    ASSERT(btree_count(tree) == n, "all keys inserted");

    /* Delete all keys in random order */
    shuffle(keys, n);
    for (int i = 0; i < n; i++) {
        btree_delete(tree, keys[i]);
        ASSERT(btree_validate(tree), "tree valid during deletion");
    }

    ASSERT(btree_count(tree) == 0, "tree empty after deleting all");
    ASSERT(tree->root->is_leaf, "root is leaf when empty");

    btree_destroy(tree);
}

static void test_delete_nonexistent(void) {
    TEST("Delete Non-existent Key");

    BTree *tree = btree_create(3);

    for (int i = 1; i <= 10; i++) {
        btree_insert(tree, i * 2);  /* Insert even numbers */
    }

    int count_before = btree_count(tree);

    /* Try to delete keys that don't exist */
    btree_delete(tree, 100);  /* Too large */
    btree_delete(tree, 0);    /* Too small */
    btree_delete(tree, 5);    /* In range but not present */

    ASSERT(btree_count(tree) == count_before, "count unchanged");
    ASSERT(btree_validate(tree), "tree still valid");

    btree_destroy(tree);
}

static void test_root_shrink(void) {
    TEST("Root Shrink (Height Decrease)");

    BTree *tree = btree_create(2);

    /* Insert keys to create multiple levels */
    for (int i = 1; i <= 7; i++) {
        btree_insert(tree, i);
    }

    int height_before = btree_height(tree);
    ASSERT(height_before > 1, "tree has multiple levels");

    /* Delete enough keys to cause root shrink */
    for (int i = 1; i <= 5; i++) {
        btree_delete(tree, i);
    }

    ASSERT(btree_validate(tree), "tree valid after root shrink");
    ASSERT(btree_height(tree) < height_before, "height decreased");

    btree_destroy(tree);
}

/* ================================================================
 * STRESS TESTS
 * ================================================================ */

static void test_large_sequential(void) {
    TEST("Large Sequential Insert/Delete");

    BTree *tree = btree_create(50);
    int n = 10000;

    /* Sequential insert */
    for (int i = 0; i < n; i++) {
        btree_insert(tree, i);
    }
    ASSERT(btree_count(tree) == n, "all keys inserted");
    ASSERT(btree_validate(tree), "tree valid after inserts");

    /* Sequential delete */
    for (int i = 0; i < n; i++) {
        btree_delete(tree, i);
    }
    ASSERT(btree_count(tree) == 0, "all keys deleted");
    ASSERT(btree_validate(tree), "tree valid after all deletes");

    btree_destroy(tree);
}

static void test_large_random(void) {
    TEST("Large Random Insert/Delete");

    BTree *tree = btree_create(50);
    int n = 10000;
    int *keys = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        keys[i] = i;
    }
    shuffle(keys, n);

    /* Random insert */
    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }
    ASSERT(btree_count(tree) == n, "all keys inserted");
    ASSERT(btree_validate(tree), "tree valid after random inserts");

    /* Random delete */
    shuffle(keys, n);
    for (int i = 0; i < n; i++) {
        btree_delete(tree, keys[i]);
        /* Validate periodically to save time */
        if (i % 1000 == 0) {
            ASSERT(btree_validate(tree), "tree valid during random deletes");
        }
    }
    ASSERT(btree_count(tree) == 0, "all keys deleted");
    ASSERT(btree_validate(tree), "tree valid after all random deletes");

    free(keys);
    btree_destroy(tree);
}

/* ================================================================
 * PERFORMANCE BENCHMARKS
 *
 * Uses clock_gettime(CLOCK_MONOTONIC) for portable wall-clock timing.
 * Runs multiple iterations and reports mean with standard deviation
 * to expose measurement instability on small inputs.
 * ================================================================ */

#ifdef __APPLE__
#include <mach/mach_time.h>
static double get_time_ns(void) {
    static mach_timebase_info_data_t info = {0, 0};
    if (info.denom == 0) {
        mach_timebase_info(&info);
    }
    return (double)mach_absolute_time() * info.numer / info.denom;
}
#else
static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}
#endif

#define BENCH_ITERATIONS 5
#define MIN_TIME_NS 1e6  /* 1ms minimum to reduce noise */

#include <math.h>

typedef struct {
    double mean_ms;
    double stddev_ms;
    double ops_per_sec;
} BenchResult;

/*
 * Run a benchmark function multiple times and compute statistics.
 * For small n, we repeat the inner loop to ensure meaningful timing.
 */
static BenchResult run_benchmark(void (*setup)(BTree *tree, int *keys, int n),
                                  void (*operation)(BTree *tree, int *keys, int n),
                                  int n, int t) {
    double times[BENCH_ITERATIONS];

    for (int iter = 0; iter < BENCH_ITERATIONS; iter++) {
        BTree *tree = btree_create(t);
        int *keys = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            keys[i] = i;
        }
        shuffle(keys, n);

        if (setup) {
            setup(tree, keys, n);
        }

        double start = get_time_ns();
        operation(tree, keys, n);
        double elapsed_ns = get_time_ns() - start;

        times[iter] = elapsed_ns / 1e6;  /* Convert to ms */

        free(keys);
        btree_destroy(tree);
    }

    /* Compute mean */
    double sum = 0;
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        sum += times[i];
    }
    double mean = sum / BENCH_ITERATIONS;

    /* Compute standard deviation */
    double sq_diff_sum = 0;
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        double diff = times[i] - mean;
        sq_diff_sum += diff * diff;
    }
    double stddev = sqrt(sq_diff_sum / BENCH_ITERATIONS);

    BenchResult result = {
        .mean_ms = mean,
        .stddev_ms = stddev,
        .ops_per_sec = (mean > 0) ? (n / mean * 1000.0) : 0
    };
    return result;
}

/* Benchmark operation functions */
static void op_insert(BTree *tree, int *keys, int n) {
    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }
}

static void setup_for_search(BTree *tree, int *keys, int n) {
    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }
    shuffle(keys, n);
}

static void op_search(BTree *tree, int *keys, int n) {
    for (int i = 0; i < n; i++) {
        btree_search(tree->root, keys[i], NULL);
    }
}

static void setup_for_delete(BTree *tree, int *keys, int n) {
    for (int i = 0; i < n; i++) {
        btree_insert(tree, keys[i]);
    }
    shuffle(keys, n);
}

static void op_delete(BTree *tree, int *keys, int n) {
    for (int i = 0; i < n; i++) {
        btree_delete(tree, keys[i]);
    }
}

static void benchmark_insert(int n, int t) {
    BenchResult r = run_benchmark(NULL, op_insert, n, t);
    printf("  Insert %6d (t=%3d): %7.2f ms (±%.2f) %10.0f ops/sec\n",
           n, t, r.mean_ms, r.stddev_ms, r.ops_per_sec);
}

static void benchmark_search(int n, int t) {
    BenchResult r = run_benchmark(setup_for_search, op_search, n, t);
    printf("  Search %6d (t=%3d): %7.2f ms (±%.2f) %10.0f ops/sec\n",
           n, t, r.mean_ms, r.stddev_ms, r.ops_per_sec);
}

static void benchmark_delete(int n, int t) {
    BenchResult r = run_benchmark(setup_for_delete, op_delete, n, t);
    printf("  Delete %6d (t=%3d): %7.2f ms (±%.2f) %10.0f ops/sec\n",
           n, t, r.mean_ms, r.stddev_ms, r.ops_per_sec);
}

static void benchmark_mixed(int n, int t) {
    double times[BENCH_ITERATIONS];

    for (int iter = 0; iter < BENCH_ITERATIONS; iter++) {
        BTree *tree = btree_create(t);
        int *keys = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            keys[i] = i;
        }
        shuffle(keys, n);

        /* Insert half as setup */
        for (int i = 0; i < n / 2; i++) {
            btree_insert(tree, keys[i]);
        }

        double start = get_time_ns();
        /* Mixed operations: insert, search, delete */
        for (int i = 0; i < n / 2; i++) {
            btree_insert(tree, keys[n / 2 + i]);
            btree_search(tree->root, keys[i], NULL);
            btree_delete(tree, keys[i]);
        }
        times[iter] = (get_time_ns() - start) / 1e6;

        free(keys);
        btree_destroy(tree);
    }

    double sum = 0, sq_sum = 0;
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        sum += times[i];
    }
    double mean = sum / BENCH_ITERATIONS;
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        double diff = times[i] - mean;
        sq_sum += diff * diff;
    }
    double stddev = sqrt(sq_sum / BENCH_ITERATIONS);

    int ops = (n / 2) * 3;
    double ops_per_sec = (mean > 0) ? (ops / mean * 1000.0) : 0;
    printf("  Mixed  %6d (t=%3d): %7.2f ms (±%.2f) %10.0f ops/sec\n",
           ops, t, mean, stddev, ops_per_sec);
}

static void run_benchmarks(void) {
    printf("\n===== PERFORMANCE BENCHMARKS =====\n");
    printf("(Running %d iterations per benchmark, showing mean ± stddev)\n",
           BENCH_ITERATIONS);

    int sizes[] = {1000, 10000, 100000};
    int degrees[] = {2, 10, 50, 100};

    printf("\n--- Varying Size (t=50) ---\n");
    for (int i = 0; i < 3; i++) {
        benchmark_insert(sizes[i], 50);
        benchmark_search(sizes[i], 50);
        benchmark_delete(sizes[i], 50);
        if (i < 2) printf("\n");
    }

    printf("\n--- Varying Minimum Degree (n=50000) ---\n");
    for (int i = 0; i < 4; i++) {
        benchmark_insert(50000, degrees[i]);
        benchmark_search(50000, degrees[i]);
        benchmark_delete(50000, degrees[i]);
        if (i < 3) printf("\n");
    }

    printf("\n--- Mixed Workload (n=50000) ---\n");
    for (int i = 0; i < 4; i++) {
        benchmark_mixed(50000, degrees[i]);
    }
}

/* ================================================================
 * MAIN
 * ================================================================ */

static void run_tests(void) {
    printf("===== B-TREE FUNCTIONALITY TESTS =====\n");

    /* Basic tests */
    test_create_destroy();
    test_insert_search();

    /* Delete tests */
    test_delete_from_leaf();
    test_delete_with_predecessor();
    test_delete_with_successor();
    test_delete_with_merge();
    test_delete_with_borrow();
    test_delete_all_keys();
    test_delete_nonexistent();
    test_root_shrink();

    /* Stress tests */
    test_large_sequential();
    test_large_random();

    printf("\n===== TEST SUMMARY =====\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    if (argc > 1 && strcmp(argv[1], "--bench") == 0) {
        run_benchmarks();
    } else if (argc > 1 && strcmp(argv[1], "--all") == 0) {
        run_tests();
        run_benchmarks();
    } else {
        run_tests();
    }

    return tests_failed > 0 ? 1 : 0;
}
