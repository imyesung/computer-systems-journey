#include <stdio.h>
#include <stdlib.h>
#include "avl_tree.h"

#define NIL_HEIGHT (-1)

/* ---------- Internal Helpers (Static) ---------- */

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

/* rotate_left/right_raw: raw pointer shuffles, no height/bf logic */
static struct Node *rotate_left_raw(struct Node *x) {
    struct Node *y = x->right;
    struct Node *B = y->left;

    y->left = x;
    x->right = B;

    return y;
}

static struct Node *rotate_right_raw(struct Node *y) {
    struct Node *x = y->left;
    struct Node *B = x->right;

    x->right = y;
    y->left  = B;

    return x;
}

// TODO: rebalance - restore AVL invariants (updates height + runs LL/LR/RL/RR rotations)
static struct Node *rebalance(struct Node *node) {
    if (node == NULL) return NULL;
    // ----------------------------------------------------------------------
    //   - Precondition:
    //       * `node` is the root of a subtree whose left and right children
    //         are already valid AVL trees with correct `height` fields.
    //       * The BST ordering invariant is satisfied for `node` and its children.
    //   - Behavior:
    //       * Recompute `node`'s height from its children
    //         (e.g., via `updateHeight(node)`).
    //       * Compute balance factor bf = height(node->left) - height(node->right).
    //       * If bf ∈ {-1, 0, +1}, subtree is AVL-balanced.
    //       * If bf == +2 (left heavy) -> LL/LR
    //       * If bf == -2 (right heavy) -> RR/RL
    //       * Use the low-level `rotate_left_raw` / `rotate_right_raw` helpers
    //         (or wrappers around them) to perform rotations.
    //       * After rotations, recompute heights of all nodes that changed
    //         and return the new root pointer of this local subtree.
    //
    //   - Postcondition:
    //       * The returned pointer is the root of a subtree that satisfies
    //         both the BST invariant and the AVL balance condition
    //         (|bf| <= 1 at every node in this subtree).
    // ----------------------------------------------------------------------
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

struct Node *insertBST(struct Node *node, int key) {
    if (node == NULL) {
        return newNode(key);
    }
    if (key < node->key) {
        node->left = insertBST(node->left, key);
    } else if (key > node->key) {
        node->right = insertBST(node->right, key);
    } else {
        return node;  // duplicate key: do nothing
    }

    updateHeight(node);
    return node;
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

    return check_range(n->left,  min_ok,     n->key, has_min, 1) &&
           check_range(n->right, n->key, max_ok,     1,       has_max);
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
