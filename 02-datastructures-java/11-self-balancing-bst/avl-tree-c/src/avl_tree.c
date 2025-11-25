#include <stdio.h>
#include <stdlib.h>
#include "avl_tree.h"

#define NIL_HEIGHT (-1)
#define TRACE_ROTATIONS 1

/* ---------- Internal Helpers (Static) ---------- */

static int getHeight(struct Node *n);
static void updateHeight(struct Node *n);

#if TRACE_ROTATIONS
static void log_trigger(const char *reason, struct Node *node, int bf) {
    printf("[rebalance trigger] %s at node %d (bf=%+d)\n",
           reason,
           node ? node->key : -999,
           bf);
}

static void log_rotation(const char *label, int pivot_key, int new_root_key) {
    printf("[rotation] %-7s pivot=%d -> new_root=%d\n",
           label,
           pivot_key,
           new_root_key);
}

static void log_state(const char *phase, struct Node *node) {
    if (node == NULL) {
        printf("[state:%s] <null>\n", phase);
        return;
    }

    int hl = getHeight(node->left);
    int hr = getHeight(node->right);
    int bf = hl - hr;

    printf("[state:%s] node=%d h=%d hl=%d hr=%d bf=%+d L=%s R=%s\n",
           phase,
           node->key,
           node->height,
           hl,
           hr,
           bf,
           node->left ? "X" : ".",
           node->right ? "X" : ".");
}
#else
#define log_trigger(reason, node, bf) ((void)0)
#define log_rotation(label, pivot, new_root) ((void)0)
#define log_state(phase, node) ((void)0)
#endif

static struct Node *newNode(int key) {
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 0;  // leaf height is considered 0
    return node;
}

/* Safe accessor for height */
static int getHeight(struct Node *n) {
    if (n == NULL) return NIL_HEIGHT;
    return n->height;
}

/* Utility to get maximum of two integers */
static int max(int a, int b) {
    return (a > b) ? a : b;
}

/* updateHeight: Must be called after any modification */
static void updateHeight(struct Node *n) {
    if (n != NULL) {
        n->height = 1 + max(getHeight(n->left), getHeight(n->right));
    }
}

/* getBalanceFactor: Left Height - Right Height */
static int getBalanceFactor(struct Node *n) {
    if (n == NULL) return 0;
    return getHeight(n->left) - getHeight(n->right);
}

/* minValueNode: return the leftmost node */
static struct Node *minValueNode(struct Node *node) {
    struct Node *current = node;
    while (current != NULL && current->left != NULL) {
        current = current->left;
    }
    return current;
}

static struct Node *rotate_left(struct Node *x) {
    struct Node *y = x->right;
    struct Node *B = (y != NULL) ? y->left : NULL;

    if (y == NULL) return x;  // nothing to rotate

    y->left = x;
    x->right = B;

    updateHeight(x);
    updateHeight(y);
    return y;
}

static struct Node *rotate_right(struct Node *y) {
    struct Node *x = y->left;
    struct Node *B = (x != NULL) ? x->right : NULL;

    if (x == NULL) return y;  // nothing to rotate

    x->right = y;
    y->left  = B;

    updateHeight(y);
    updateHeight(x);
    return x;
}

/* rebalance: check balance factor and perform rotations as needed */
static struct Node *rebalance(struct Node *node) {
    if (node == NULL) return NULL;

    updateHeight(node);
    log_state("before", node);
    int bf = getBalanceFactor(node);

    if (bf > 1) {  // left subtree heavier than right
        log_trigger("left-heavy", node, bf);
        if (getBalanceFactor(node->left) < 0) {  // LR pattern
            int pivot_key = node->left ? node->left->key : -999;
            int new_root_key = (node->left && node->left->right)
                                   ? node->left->right->key
                                   : -999;
            log_rotation("LR-pre", pivot_key, new_root_key);
            node->left = rotate_left(node->left);
            log_state("after-child-rot", node->left);
        }
        int pivot_key = node->key;
        int new_root_key = node->left ? node->left->key : -999;
        struct Node *new_root = rotate_right(node);
        log_rotation("LL", pivot_key, new_root_key);
        log_state("after-root-rot", new_root);
        return new_root;
    }

    if (bf < -1) {  // right subtree heavier than left
        log_trigger("right-heavy", node, bf);
        if (getBalanceFactor(node->right) > 0) {  // RL pattern
            int pivot_key = node->right ? node->right->key : -999;
            int new_root_key = (node->right && node->right->left)
                                   ? node->right->left->key
                                   : -999;
            log_rotation("RL-pre", pivot_key, new_root_key);
            node->right = rotate_right(node->right);
            log_state("after-child-rot", node->right);
        }
        int pivot_key = node->key;
        int new_root_key = node->right ? node->right->key : -999;
        struct Node *new_root = rotate_left(node);
        log_rotation("RR", pivot_key, new_root_key);
        log_state("after-root-rot", new_root);
        return new_root;
    }

    log_state("after-no-rot", node);
    return node;
}

/* ---------- Public Operations Implementation ---------- */

int searchBST(struct Node *root, int key) {
    if (root == NULL) return 0;
    if (key == root->key) return 1;
    return (key < root->key)
        ? searchBST(root->left, key)
        : searchBST(root->right, key);
}

struct Node *insertBST(struct Node *root, int key) {
    if (root == NULL) {
        return newNode(key);
    }
    if (key < root->key) {
        root->left = insertBST(root->left, key);
    } else if (key > root->key) {
        root->right = insertBST(root->right, key);
    } else {
        return root;  // duplicate key: do nothing
    }

    updateHeight(root);
    return root;
}

/* TODO: insertAVL - BST insert + rebalance on return */
struct Node *insertAVL(struct Node *root, int key) {
    (void)key;
    return root;
}

struct Node *deleteBST(struct Node *root, int key) {
    if (root == NULL) {
        return NULL;
    }

    if (key < root->key) {
        root->left = deleteBST(root->left, key);
    } else if (key > root->key) {
        root->right = deleteBST(root->right, key);
    } else {
        /* key == root->key: delete this node */
        if (root->left == NULL && root->right == NULL) {
            free(root);
            return NULL;
        }

        if (root->left == NULL || root->right == NULL) {
            struct Node *child = (root->left != NULL) ? root->left : root->right;
            free(root);
            return child;
        }

        struct Node *successor = minValueNode(root->right);
        root->key = successor->key;
        root->right = deleteBST(root->right, successor->key);
    }

    if (root != NULL) {
        updateHeight(root);
    }

    return root;
}

/* TODO: deleteAVL - BST delete + rebalance on return */
struct Node *deleteAVL(struct Node *root, int key) {
    (void)key;
    return root;
}

void freeTree(struct Node *root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

/* ---------- Traversal & Debug Implementation ---------- */

void print_inorder(struct Node *root) {
    if (root == NULL) return;
    print_inorder(root->left);
    printf("%d ", root->key);
    print_inorder(root->right);
}

#define INDENT_STEP 4
#define COMMENT_COL 32

void print_tree_debug(struct Node *n, int depth) {
    if (n == NULL) return;

    print_tree_debug(n->right, depth + 1);

    for (int i = 0; i < depth * INDENT_STEP; i++) {
        putchar(' ');
    }

    printf("%4d", n->key);

    int current_col = depth * INDENT_STEP + 4;
    int spaces = COMMENT_COL - current_col;
    if (spaces < 1) spaces = 1;

    for (int i = 0; i < spaces; i++) {
        putchar(' ');
    }

    int h  = getHeight(n);
    int bf = getBalanceFactor(n);

    if (abs(bf) > 1) {
        printf("(h=%2d, bf=%+2d)  !!\n", h, bf);
    } else {
        printf("(h=%2d, bf=%+2d)\n", h, bf);
    }

    print_tree_debug(n->left, depth + 1);
}

/* ---------- Invariant Checkers Implementation ---------- */

static int check_range(struct Node *n, int min_ok, int max_ok, int has_min, int has_max) {
    if (n == NULL) return 1;

    if ((has_min && n->key <= min_ok) ||
        (has_max && n->key >= max_ok)) {
        return 0;
    }

    return check_range(n->left, min_ok, n->key, has_min, 1) &&
           check_range(n->right, n->key, max_ok, 1, has_max);
}

static int check_bst_invariant(struct Node *root) {
    return check_range(root, 0, 0, 0, 0);
}

static int check_avl_subtree(struct Node *n, int *out_height) {
    if (n == NULL) {
        *out_height = NIL_HEIGHT;
        return 1;
    }

    int hl = 0;
    int hr = 0;

    if (!check_avl_subtree(n->left, &hl)) return 0;
    if (!check_avl_subtree(n->right, &hr)) return 0;

    int expected_height = 1 + max(hl, hr);
    if (n->height != expected_height) {
        fprintf(stderr,
                "Error: Node %d has stored height %d, but real height is %d\n",
                n->key, n->height, expected_height);
        return 0;
    }

    int bf = hl - hr;
    if (bf < -1 || bf > 1) {
        fprintf(stderr, "Error: Node %d is unbalanced (BF = %d)\n", n->key, bf);
        return 0;
    }

    *out_height = expected_height;
    return 1;
}

int check_avl_invariant(struct Node *root) {
    int h = 0;
    if (!check_avl_subtree(root, &h)) {
        return 0;
    }
    if (!check_bst_invariant(root)) {
        fprintf(stderr, "Error: BST property violated (order mismatch)\n");
        return 0;
    }
    return 1;
}
