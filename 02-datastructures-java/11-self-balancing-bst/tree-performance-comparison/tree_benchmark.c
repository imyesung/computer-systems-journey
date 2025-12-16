/*
 * Tree Performance Benchmark
 * Splay Tree, AVL Tree, B-Tree insert() comparison
 * 
 * Compile: gcc -O2 -o tree_benchmark tree_benchmark.c -lm
 * Usage: ./tree_benchmark <tree_type> <pattern> <n> <trials>
 *        tree_type: splay | avl | btree
 *        pattern: random | sorted | reverse
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/* ============================================================
 * SPLAY TREE
 * ============================================================ */
typedef struct SplayNode {
    int key;
    struct SplayNode *left, *right;
} SplayNode;

SplayNode* splay_new_node(int key) {
    SplayNode* node = (SplayNode*)malloc(sizeof(SplayNode));
    node->key = key;
    node->left = node->right = NULL;
    return node;
}

/* Right rotation */
SplayNode* splay_rotate_right(SplayNode* x) {
    SplayNode* y = x->left;
    x->left = y->right;
    y->right = x;
    return y;
}

/* Left rotation */
SplayNode* splay_rotate_left(SplayNode* x) {
    SplayNode* y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

/* Splay operation: bring key to root */
SplayNode* splay(SplayNode* root, int key) {
    if (root == NULL || root->key == key)
        return root;

    if (key < root->key) {
        if (root->left == NULL) return root;

        if (key < root->left->key) {
            /* Zig-Zig (Left Left) */
            root->left->left = splay(root->left->left, key);
            root = splay_rotate_right(root);
        } else if (key > root->left->key) {
            /* Zig-Zag (Left Right) */
            root->left->right = splay(root->left->right, key);
            if (root->left->right != NULL)
                root->left = splay_rotate_left(root->left);
        }
        return (root->left == NULL) ? root : splay_rotate_right(root);
    } else {
        if (root->right == NULL) return root;

        if (key > root->right->key) {
            /* Zag-Zag (Right Right) */
            root->right->right = splay(root->right->right, key);
            root = splay_rotate_left(root);
        } else if (key < root->right->key) {
            /* Zag-Zig (Right Left) */
            root->right->left = splay(root->right->left, key);
            if (root->right->left != NULL)
                root->right = splay_rotate_right(root->right);
        }
        return (root->right == NULL) ? root : splay_rotate_left(root);
    }
}

SplayNode* splay_insert(SplayNode* root, int key) {
    if (root == NULL) return splay_new_node(key);

    root = splay(root, key);

    if (root->key == key) return root;  /* Duplicate */

    SplayNode* node = splay_new_node(key);

    if (key < root->key) {
        node->right = root;
        node->left = root->left;
        root->left = NULL;
    } else {
        node->left = root;
        node->right = root->right;
        root->right = NULL;
    }
    return node;
}

void splay_free(SplayNode* root) {
    if (root) {
        splay_free(root->left);
        splay_free(root->right);
        free(root);
    }
}

/* ============================================================
 * AVL TREE
 * ============================================================ */
typedef struct AVLNode {
    int key;
    int height;
    struct AVLNode *left, *right;
} AVLNode;

int avl_height(AVLNode* n) {
    return n ? n->height : 0;
}

int avl_max(int a, int b) {
    return (a > b) ? a : b;
}

AVLNode* avl_new_node(int key) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->key = key;
    node->height = 1;
    node->left = node->right = NULL;
    return node;
}

AVLNode* avl_rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = avl_max(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = avl_max(avl_height(x->left), avl_height(x->right)) + 1;

    return x;
}

AVLNode* avl_rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = avl_max(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = avl_max(avl_height(y->left), avl_height(y->right)) + 1;

    return y;
}

int avl_get_balance(AVLNode* n) {
    return n ? avl_height(n->left) - avl_height(n->right) : 0;
}

AVLNode* avl_insert(AVLNode* node, int key) {
    if (node == NULL) return avl_new_node(key);

    if (key < node->key)
        node->left = avl_insert(node->left, key);
    else if (key > node->key)
        node->right = avl_insert(node->right, key);
    else
        return node;  /* Duplicate */

    node->height = 1 + avl_max(avl_height(node->left), avl_height(node->right));

    int balance = avl_get_balance(node);

    /* Left Left Case */
    if (balance > 1 && key < node->left->key)
        return avl_rotate_right(node);

    /* Right Right Case */
    if (balance < -1 && key > node->right->key)
        return avl_rotate_left(node);

    /* Left Right Case */
    if (balance > 1 && key > node->left->key) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }

    /* Right Left Case */
    if (balance < -1 && key < node->right->key) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }

    return node;
}

void avl_free(AVLNode* root) {
    if (root) {
        avl_free(root->left);
        avl_free(root->right);
        free(root);
    }
}

/* ============================================================
 * B-TREE (Order M = 128, optimized for cache lines)
 * ============================================================ */
#define BTREE_M 128  /* Max children per node */
#define BTREE_MIN_KEYS ((BTREE_M - 1) / 2)

typedef struct BTreeNode {
    int keys[BTREE_M - 1];
    struct BTreeNode* children[BTREE_M];
    int n;          /* Number of keys */
    int leaf;       /* 1 if leaf */
} BTreeNode;

typedef struct BTree {
    BTreeNode* root;
} BTree;

BTreeNode* btree_create_node(int leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    node->n = 0;
    node->leaf = leaf;
    for (int i = 0; i < BTREE_M; i++)
        node->children[i] = NULL;
    return node;
}

BTree* btree_create(void) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    tree->root = btree_create_node(1);
    return tree;
}

void btree_split_child(BTreeNode* parent, int i, BTreeNode* full_child) {
    int mid = (BTREE_M - 1) / 2;
    BTreeNode* new_node = btree_create_node(full_child->leaf);
    new_node->n = BTREE_M - 1 - mid - 1;

    /* Copy right half of keys to new node */
    for (int j = 0; j < new_node->n; j++)
        new_node->keys[j] = full_child->keys[mid + 1 + j];

    /* Copy right half of children if not leaf */
    if (!full_child->leaf) {
        for (int j = 0; j <= new_node->n; j++)
            new_node->children[j] = full_child->children[mid + 1 + j];
    }

    full_child->n = mid;

    /* Shift parent's children right */
    for (int j = parent->n; j >= i + 1; j--)
        parent->children[j + 1] = parent->children[j];
    parent->children[i + 1] = new_node;

    /* Shift parent's keys right */
    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = full_child->keys[mid];
    parent->n++;
}

void btree_insert_nonfull(BTreeNode* node, int key) {
    int i = node->n - 1;

    if (node->leaf) {
        /* Shift keys and insert */
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->n++;
    } else {
        /* Find child to descend into */
        while (i >= 0 && key < node->keys[i])
            i--;
        i++;

        if (node->children[i]->n == BTREE_M - 1) {
            btree_split_child(node, i, node->children[i]);
            if (key > node->keys[i])
                i++;
        }
        btree_insert_nonfull(node->children[i], key);
    }
}

void btree_insert(BTree* tree, int key) {
    BTreeNode* root = tree->root;

    if (root->n == BTREE_M - 1) {
        BTreeNode* new_root = btree_create_node(0);
        new_root->children[0] = root;
        btree_split_child(new_root, 0, root);

        int i = (new_root->keys[0] < key) ? 1 : 0;
        btree_insert_nonfull(new_root->children[i], key);

        tree->root = new_root;
    } else {
        btree_insert_nonfull(root, key);
    }
}

void btree_free_node(BTreeNode* node) {
    if (node) {
        if (!node->leaf) {
            for (int i = 0; i <= node->n; i++)
                btree_free_node(node->children[i]);
        }
        free(node);
    }
}

void btree_free(BTree* tree) {
    btree_free_node(tree->root);
    free(tree);
}

/* ============================================================
 * BENCHMARK HARNESS
 * ============================================================ */

/* Fisher-Yates shuffle */
void shuffle(int* arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

/* Generate input array based on pattern */
void generate_input(int* arr, int n, const char* pattern) {
    for (int i = 0; i < n; i++)
        arr[i] = i;

    if (strcmp(pattern, "random") == 0) {
        shuffle(arr, n);
    } else if (strcmp(pattern, "reverse") == 0) {
        for (int i = 0; i < n; i++)
            arr[i] = n - 1 - i;
    }
    /* "sorted" keeps array as-is */
}

/* High-resolution timing */
double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

double benchmark_splay(int* arr, int n) {
    double start = get_time_sec();
    SplayNode* root = NULL;
    for (int i = 0; i < n; i++)
        root = splay_insert(root, arr[i]);
    double end = get_time_sec();
    splay_free(root);
    return end - start;
}

double benchmark_avl(int* arr, int n) {
    double start = get_time_sec();
    AVLNode* root = NULL;
    for (int i = 0; i < n; i++)
        root = avl_insert(root, arr[i]);
    double end = get_time_sec();
    avl_free(root);
    return end - start;
}

double benchmark_btree(int* arr, int n) {
    double start = get_time_sec();
    BTree* tree = btree_create();
    for (int i = 0; i < n; i++)
        btree_insert(tree, arr[i]);
    double end = get_time_sec();
    btree_free(tree);
    return end - start;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <tree_type> <pattern> <n> <trials>\n", argv[0]);
        return 1;
    }

    const char* tree_type = argv[1];
    const char* pattern = argv[2];
    int n = atoi(argv[3]);
    int trials = atoi(argv[4]);

    int* arr = (int*)malloc(n * sizeof(int));

    for (int t = 0; t < trials; t++) {
        srand(42 + t);  /* Reproducible randomness */
        generate_input(arr, n, pattern);

        double elapsed;
        if (strcmp(tree_type, "splay") == 0)
            elapsed = benchmark_splay(arr, n);
        else if (strcmp(tree_type, "avl") == 0)
            elapsed = benchmark_avl(arr, n);
        else if (strcmp(tree_type, "btree") == 0)
            elapsed = benchmark_btree(arr, n);
        else {
            fprintf(stderr, "Unknown tree type: %s\n", tree_type);
            free(arr);
            return 1;
        }

        printf("%.9f\n", elapsed);
    }

    free(arr);
    return 0;
}
