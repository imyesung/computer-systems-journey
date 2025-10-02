// bst_vs_array_benchmark.c
// Single-file C benchmark: BST (iterative/recursive) vs array linear search
// - High-resolution timer (clock_gettime MONOTONIC)
// - Dead-code elimination prevention via a volatile sink
// - Skewed vs balanced demo
// - CLI: --size N --queries Q --seed S --demoN K
// - FIXED recursive section: builds its own 0..REC_N-1 dataset so hits make sense

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ========================================
// Global sink to defeat dead-code elimination
// ========================================
static volatile long long g_sink = 0;

// ========================================
// High-resolution timer (ms)
// ========================================
static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

// ========================================
// Safe allocation helpers
// ========================================
static void* xmalloc(size_t n) {
    void* p = malloc(n);
    if (!p) {
        fprintf(stderr, "malloc failed (requested %zu bytes)\n", n);
        exit(EXIT_FAILURE);
    }
    return p;
}

// ========================================
// BST Node
// ========================================
typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

static Node* create_node(int value) {
    Node* p = (Node*)xmalloc(sizeof(Node));
    p->data = value;
    p->left = p->right = NULL;
    return p;
}

// ========================================
// BST Operations — Recursive
// ========================================
static Node* insert_recursive(Node* root, int value) {
    if (root == NULL) return create_node(value);
    if (value < root->data) {
        root->left = insert_recursive(root->left, value);
    } else if (value > root->data) {
        root->right = insert_recursive(root->right, value);
    } // duplicates ignored
    return root;
}

static Node* search_recursive(Node* root, int value) {
    if (root == NULL || root->data == value) return root;
    if (value < root->data) return search_recursive(root->left, value);
    return search_recursive(root->right, value);
}

// ========================================
// BST Operations — Iterative (stack-safe)
// ========================================
static Node* insert_iterative(Node* root, int value) {
    if (root == NULL) return create_node(value);

    Node* cur = root;
    Node* parent = NULL;

    while (cur != NULL) {
        parent = cur;
        if (value < cur->data) {
            cur = cur->left;
        } else if (value > cur->data) {
            cur = cur->right;
        } else {
            // duplicate: do nothing
            return root;
        }
    }

    Node* n = create_node(value);
    if (value < parent->data) parent->left = n;
    else parent->right = n;

    return root;
}

static Node* search_iterative(Node* root, int value) {
    Node* cur = root;
    while (cur && cur->data != value) {
        cur = (value < cur->data) ? cur->left : cur->right;
    }
    return cur;
}

// ========================================
// Utilities: linear search, shuffle, height, free
// ========================================
static int linear_search(const int* arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) return i;
    }
    return -1;
}

static void shuffle(int* arr, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

static int get_tree_height(const Node* root) {
    if (!root) return 0;
    int lh = get_tree_height(root->left);
    int rh = get_tree_height(root->right);
    return 1 + (lh > rh ? lh : rh);
}

static void free_tree(Node* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

// ========================================
// CLI args
// ========================================
typedef struct {
    int size;         // number of elements
    int queries;      // number of search queries
    int seed;         // RNG seed (0 => time-based)
    int demo_small_N; // small N for skew vs balanced demo
} BenchArgs;

static BenchArgs parse_args(int argc, char** argv) {
    BenchArgs a = { .size = 1000000, .queries = 20000, .seed = 0, .demo_small_N = 100 };
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--size") && i + 1 < argc) {
            a.size = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--queries") && i + 1 < argc) {
            a.queries = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--seed") && i + 1 < argc) {
            a.seed = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--demoN") && i + 1 < argc) {
            a.demo_small_N = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            printf("Usage: %s [--size N] [--queries Q] [--seed S] [--demoN K]\n", argv[0]);
            exit(0);
        }
    }
    return a;
}

// ========================================
// Main
// ========================================
int main(int argc, char** argv) {
    BenchArgs cfg = parse_args(argc, argv);

    if (cfg.seed == 0) cfg.seed = (int)time(NULL);
    srand((unsigned)cfg.seed);

    printf("BST vs Array Performance Benchmark\n");
    printf("  size    : %d\n", cfg.size);
    printf("  queries : %d\n", cfg.queries);
    printf("  seed    : %d\n\n", cfg.seed);

    // -------------------------------------------------------------------------
    // Experiment 1: Skewed vs Balanced (small N demo)
    // -------------------------------------------------------------------------
    printf("=== Experiment 1: Skewed vs Balanced (N=%d) ===\n", cfg.demo_small_N);

    int smallN = cfg.demo_small_N;
    int* seq = (int*)xmalloc(sizeof(int) * smallN);
    int* shf = (int*)xmalloc(sizeof(int) * smallN);
    for (int i = 0; i < smallN; i++) {
        seq[i] = i;
        shf[i] = i;
    }
    shuffle(shf, smallN);

    Node* skew = NULL;
    for (int i = 0; i < smallN; i++) {
        skew = insert_iterative(skew, seq[i]); // sequential -> skewed
    }
    Node* bal = NULL;
    for (int i = 0; i < smallN; i++) {
        bal = insert_iterative(bal, shf[i]); // shuffled -> balanced-ish
    }

    int h_skew = get_tree_height(skew);
    int h_bal  = get_tree_height(bal);
    printf("Skewed (sequential insert)  height: %d  (worst-case ~N)\n", h_skew);
    printf("Balanced (shuffled insert)  height: %d  (~log N expected)\n\n", h_bal);

    free_tree(skew);
    free_tree(bal);
    free(seq);
    free(shf);

    // -------------------------------------------------------------------------
    // Experiment 2: Large-scale performance
    // -------------------------------------------------------------------------
    printf("=== Experiment 2: Large-scale Performance ===\n");
    const int N = cfg.size;
    const int Q = cfg.queries;

    // Data for array and BST(iter)
    int* data = (int*)xmalloc(sizeof(int) * N);
    for (int i = 0; i < N; i++) data[i] = i;

    printf("Shuffling %d integers...\n", N);
    shuffle(data, N);

    int* queries = (int*)xmalloc(sizeof(int) * Q);
    for (int i = 0; i < Q; i++) queries[i] = rand() % N;

    // 1) Array O(n) linear search
    {
        // warm-up
        for (int i = 0; i < Q; i++) (void)linear_search(data, N, queries[i]);

        long long hits = 0;
        double t0 = now_ms();
        for (int i = 0; i < Q; i++) {
            int idx = linear_search(data, N, queries[i]);
            hits += (idx >= 0);
        }
        double t1 = now_ms();
        g_sink += hits;
        printf("\n[Array O(n)] Searching %d queries...\n", Q);
        printf("Array hits: %lld\n", hits);
        printf("Array linear search time: %.3f ms\n", (t1 - t0));
    }

    // 2) BST O(log n) iterative build + search
    Node* bst = NULL;
    double t_build0 = now_ms();
    for (int i = 0; i < N; i++) {
        bst = insert_iterative(bst, data[i]);
    }
    double t_build1 = now_ms();
    double bst_build_ms = (t_build1 - t_build0);

    int bst_h = get_tree_height(bst);
    printf("\n[BST O(log n) - Iterative] Building tree with %d nodes...\n", N);
    printf("BST build time          : %.3f ms\n", bst_build_ms);
    printf("BST height              : %d\n", bst_h);

    {
        // warm-up
        for (int i = 0; i < Q; i++) (void)search_iterative(bst, queries[i]);

        long long hits = 0;
        double t0 = now_ms();
        for (int i = 0; i < Q; i++) {
            hits += (search_iterative(bst, queries[i]) != NULL);
        }
        double t1 = now_ms();
        g_sink += hits;
        printf("[BST O(log n) - Iterative] Searching %d queries...\n", Q);
        printf("BST(iter) hits: %lld\n", hits);
        printf("BST(iter) search time   : %.3f ms\n", (t1 - t0));
    }

    // 3) Recursive build + search (FIXED): build from 0..REC_N-1
    const int REC_N = (N < 200000 ? N : 200000);
    int* rec_data = (int*)xmalloc(sizeof(int) * REC_N);
    for (int i = 0; i < REC_N; i++) rec_data[i] = i;    // 0..REC_N-1
    shuffle(rec_data, REC_N);                            // avoid skew for fair depth

    Node* bst_rec = NULL;
    double t_rec_b0 = now_ms();
    for (int i = 0; i < REC_N; i++) {
        bst_rec = insert_recursive(bst_rec, rec_data[i]);
    }
    double t_rec_b1 = now_ms();
    int bst_rec_h = get_tree_height(bst_rec);
    printf("\n[BST (Recursive) — build on 0..%d shuffled] Building...\n", REC_N - 1);
    printf("BST(rec) build time     : %.3f ms\n", (t_rec_b1 - t_rec_b0));
    printf("BST(rec) height         : %d\n", bst_rec_h);

    {
        // Make queries in 0..REC_N-1 so they match the recursive tree's keyset
        int* queries_rec = (int*)xmalloc(sizeof(int) * Q);
        for (int i = 0; i < Q; i++) queries_rec[i] = rand() % REC_N;

        // warm-up
        for (int i = 0; i < Q; i++) (void)search_recursive(bst_rec, queries_rec[i]);

        long long hits = 0;
        double t0 = now_ms();
        for (int i = 0; i < Q; i++) {
            hits += (search_recursive(bst_rec, queries_rec[i]) != NULL);
        }
        double t1 = now_ms();
        g_sink += hits;
        printf("[BST (Recursive)] Searching %d queries (0..%d)...\n", Q, REC_N - 1);
        printf("BST(rec) hits: %lld\n", hits);
        printf("BST(rec) search time    : %.3f ms\n", (t1 - t0));

        free(queries_rec);
    }

    // Cleanup
    free_tree(bst);
    free_tree(bst_rec);
    free(data);
    free(rec_data);
    free(queries);

    // Use g_sink so compiler cannot drop earlier loops
    if (g_sink == 42) printf("sink=%lld\n", g_sink);

    return 0;
}
