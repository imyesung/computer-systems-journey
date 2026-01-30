/*
 * B-Tree Implementation
 * 
 * This file implements the core B-Tree operations.
 * See b-tree.h for the study checklist and B-Tree properties.
 */

#include "b-tree.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* Enable/disable trace logging for debugging */
#define TRACE_SPLIT 0
#define TRACE_DELETE 0

/* ================================================================
 * INTERNAL HELPER FUNCTIONS (Static)
 * ================================================================ */

static BTreeNode *create_node(int t, bool is_leaf);
static void destroy_node(BTreeNode *node);
static void split_child(BTreeNode *parent, int i, int t);
static void insert_non_full(BTreeNode *node, int key, int t);

/* Delete helper functions */
static int find_key(BTreeNode *node, int key);
static int get_predecessor(BTreeNode *node, int idx);
static int get_successor(BTreeNode *node, int idx);
static void merge(BTreeNode *node, int idx, int t);
static void borrow_from_left(BTreeNode *node, int idx);
static void borrow_from_right(BTreeNode *node, int idx);
static void fill(BTreeNode *node, int idx, int t);
static void delete_internal(BTreeNode *node, int key, int t);

/* Utility helper functions */
static int count_node(BTreeNode *node);

/* ---------- Trace/Debug Logging ---------- */

#if TRACE_SPLIT
static void log_split(BTreeNode *parent, int i, int median_key) {
    printf("[split] Splitting child %d of parent, median key %d promoted\n",
           i, median_key);
}

static void log_insert(int key, bool is_leaf) {
    printf("[insert] Key %d inserted into %s node\n",
           key, is_leaf ? "leaf" : "internal");
}

static void log_root_split(int median_key) {
    printf("[split] Root was full, created new root with median key %d\n",
           median_key);
}
#else
#define log_split(parent, i, median) ((void)0)
#define log_insert(key, is_leaf) ((void)0)
#define log_root_split(median) ((void)0)
#endif

#if TRACE_DELETE
static void log_delete_leaf(int key) {
    printf("[delete] Removed key %d from leaf\n", key);
}
static void log_delete_predecessor(int key, int pred) {
    printf("[delete] Replaced key %d with predecessor %d\n", key, pred);
}
static void log_delete_successor(int key, int succ) {
    printf("[delete] Replaced key %d with successor %d\n", key, succ);
}
static void log_merge(int parent_key, int idx) {
    printf("[delete] Merged children around parent key %d at index %d\n",
           parent_key, idx);
}
static void log_borrow_left(int borrowed, int idx) {
    printf("[delete] Borrowed key %d from left sibling for child %d\n",
           borrowed, idx);
}
static void log_borrow_right(int borrowed, int idx) {
    printf("[delete] Borrowed key %d from right sibling for child %d\n",
           borrowed, idx);
}
#else
#define log_delete_leaf(key) ((void)0)
#define log_delete_predecessor(key, pred) ((void)0)
#define log_delete_successor(key, succ) ((void)0)
#define log_merge(parent_key, idx) ((void)0)
#define log_borrow_left(borrowed, idx) ((void)0)
#define log_borrow_right(borrowed, idx) ((void)0)
#endif

/* ================================================================
 * NODE CREATION / DESTRUCTION
 * ================================================================ */

/*
 * create_node - Allocate and initialize a new B-Tree node
 * 
 * @t: minimum degree of the tree
 * @is_leaf: whether this node is a leaf
 * 
 * Memory layout:
 *   keys[0..2t-2]      -> max 2t-1 keys
 *   children[0..2t-1]  -> max 2t children
 * 
 * Returns: pointer to newly allocated node, or NULL on failure
 */
static BTreeNode *create_node(int t, bool is_leaf) {
    BTreeNode *node = (BTreeNode *)malloc(sizeof(BTreeNode));
    if (!node) return NULL;

    /* Allocate arrays for keys and children */
    node->keys = (int *)malloc((2 * t - 1) * sizeof(int));
    node->children = (BTreeNode **)malloc(2 * t * sizeof(BTreeNode *));
    
    if (!node->keys || !node->children) {
        free(node->keys);
        free(node->children);
        free(node);
        return NULL;
    }

    node->n = 0;
    node->is_leaf = is_leaf;

    /* Initialize all child pointers to NULL */
    for (int i = 0; i < 2 * t; i++) {
        node->children[i] = NULL;
    }

    return node;
}

/*
 * destroy_node - Recursively free a node and all its descendants
 * 
 * Post-order traversal: free children first, then the node itself
 */
static void destroy_node(BTreeNode *node) {
    if (!node) return;

    /* Recursively destroy all children first */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            destroy_node(node->children[i]);
        }
    }

    free(node->keys);
    free(node->children);
    free(node);
}

/* ================================================================
 * PUBLIC: CREATE / DESTROY
 * ================================================================ */

/*
 * btree_create - Create an empty B-Tree with given minimum degree
 * 
 * @t: minimum degree (must be >= 2)
 *     - Each node can have [t-1, 2t-1] keys
 *     - Each internal node can have [t, 2t] children
 * 
 * Common choices:
 *   t=2  -> 2-3-4 tree (each node has 1-3 keys)
 *   t=3  -> each node has 2-5 keys
 *   t=100+ -> typical for disk-based databases
 */
BTree *btree_create(int t) {
    if (t < 2) {
        fprintf(stderr, "Error: Minimum degree must be at least 2\n");
        return NULL;
    }

    BTree *tree = (BTree *)malloc(sizeof(BTree));
    if (!tree) return NULL;

    tree->t = t;
    tree->root = create_node(t, true);  /* Start with empty leaf as root */
    
    if (!tree->root) {
        free(tree);
        return NULL;
    }

    return tree;
}

/*
 * btree_destroy - Free all memory associated with the tree
 */
void btree_destroy(BTree *tree) {
    if (!tree) return;
    destroy_node(tree->root);
    free(tree);
}

/* ================================================================
 * INSERT OPERATION
 * 
 * B-Tree uses PROACTIVE splitting:
 * - Split full nodes on the way DOWN (before inserting)
 * - This ensures we never need to backtrack
 * - Contrast with AVL/RB trees which fix up on the way UP
 * ================================================================ */

/*
 * split_child - Split a full child node into two nodes
 * 
 * @parent: parent node (must have room for one more key)
 * @i: index of the full child in parent->children[]
 * @t: minimum degree
 * 
 * Before: parent->children[i] has 2t-1 keys (full)
 * After:  
 *   - parent->children[i] has t-1 keys (left half)
 *   - parent->children[i+1] has t-1 keys (right half, new node)
 *   - parent->keys[i] has the median key (promoted)
 * 
 * Visual example (t=3, full child has 5 keys):
 * 
 *     Before:                     After:
 *         [...]                      [... M ...]
 *           |                          /     \
 *     [A B M C D]                [A B]       [C D]
 *     (full: 5 keys)           (2 keys)    (2 keys)
 * 
 * The median key M is "pushed up" to the parent.
 */
static void split_child(BTreeNode *parent, int i, int t) {
    BTreeNode *full_child = parent->children[i];
    
    /* Create new node for the right half of the split */
    BTreeNode *new_child = create_node(t, full_child->is_leaf);
    new_child->n = t - 1;

    /* 
     * Copy the UPPER half of keys to new_child
     * 
     * full_child layout: [0..t-2] [t-1] [t..2t-2]
     *                    \_______/ \__/ \_______/
     *                     keep    median  copy to new
     */
    for (int j = 0; j < t - 1; j++) {
        new_child->keys[j] = full_child->keys[j + t];
    }

    /* If not a leaf, also copy the corresponding child pointers */
    if (!full_child->is_leaf) {
        for (int j = 0; j < t; j++) {
            new_child->children[j] = full_child->children[j + t];
        }
    }

    /* The original child now only has the lower t-1 keys */
    full_child->n = t - 1;

    /* 
     * Make room in parent for the new child pointer
     * Shift children[i+1..n] to children[i+2..n+1]
     */
    for (int j = parent->n; j >= i + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = new_child;

    /* 
     * Make room in parent for the median key
     * Shift keys[i..n-1] to keys[i+1..n]
     */
    for (int j = parent->n - 1; j >= i; j--) {
        parent->keys[j + 1] = parent->keys[j];
    }
    
    /* Promote the median key to parent */
    parent->keys[i] = full_child->keys[t - 1];
    parent->n++;

    log_split(parent, i, parent->keys[i]);
}

/*
 * insert_non_full - Insert a key into a node that is guaranteed not full
 * 
 * @node: node to insert into (must have n < 2t-1)
 * @key: key to insert
 * @t: minimum degree
 * 
 * Two cases:
 * 1. Leaf node: directly insert key in sorted position
 * 2. Internal node: find correct child, split if full, then recurse
 */
static void insert_non_full(BTreeNode *node, int key, int t) {
    int i = node->n - 1;  /* Start from rightmost key */

    if (node->is_leaf) {
        /* 
         * CASE 1: Leaf node
         * Find the correct position and shift keys to make room
         */
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->n++;
        
        log_insert(key, true);
    } else {
        /* 
         * CASE 2: Internal node
         * Find the child which will receive the new key
         */
        while (i >= 0 && node->keys[i] > key) {
            i--;
        }
        i++;  /* i is now the index of child to descend into */

        /* If the child is full, split it first (PROACTIVE split) */
        if (node->children[i]->n == 2 * t - 1) {
            split_child(node, i, t);
            
            /* After split, the median key is at keys[i]
             * Decide which of the two children to descend into */
            if (key > node->keys[i]) {
                i++;
            }
        }
        
        /* Recursively insert into the (possibly new) child */
        insert_non_full(node->children[i], key, t);
    }
}

/*
 * btree_insert - Insert a key into the B-Tree
 * 
 * @tree: the B-Tree
 * @key: key to insert
 * 
 * Special case: if root is full, we must create a new root first.
 * This is the ONLY case where tree height increases.
 */
void btree_insert(BTree *tree, int key) {
    if (!tree) return;

    BTreeNode *root = tree->root;

    /* Special case: root is full */
    if (root->n == 2 * tree->t - 1) {
        /* Create new root */
        BTreeNode *new_root = create_node(tree->t, false);
        new_root->children[0] = root;

        /* Split the old root */
        split_child(new_root, 0, tree->t);
        log_root_split(new_root->keys[0]);

        /* Decide which child of new root should receive the key */
        int i = 0;
        if (new_root->keys[0] < key) {
            i++;
        }
        insert_non_full(new_root->children[i], key, tree->t);

        tree->root = new_root;
    } else {
        insert_non_full(root, key, tree->t);
    }
}

/* ================================================================
 * SEARCH OPERATION (SKELETON)
 * ================================================================ */

/*
 * btree_search - Search for a key in the B-Tree
 *
 * @node: node to start search from
 * @key: key to find
 * @idx: output parameter - index of key in node if found
 *
 * Returns: pointer to node containing key, or NULL if not found
 *
 * Algorithm:
 * 1. Linear search through node->keys to find position
 * 2. If key found, return this node
 * 3. If leaf reached, key doesn't exist
 * 4. Otherwise, recurse into appropriate child
 */
BTreeNode *btree_search(BTreeNode *node, int key, int *idx) {
    if (!node) return NULL;

    /* Find the first key >= search key */
    int i = 0;
    while (i < node->n && key > node->keys[i]) {
        i++;
    }

    /* Check if we found the key */
    if (i < node->n && key == node->keys[i]) {
        if (idx) *idx = i;
        return node;
    }

    /* If this is a leaf, key doesn't exist */
    if (node->is_leaf) {
        return NULL;
    }

    /* Recurse into the appropriate child */
    return btree_search(node->children[i], key, idx);
}

/* ================================================================
 * DELETE OPERATION
 *
 * B-Tree uses PROACTIVE rebalancing on deletion:
 * - Ensure each node has at least t keys before descending
 * - This guarantees we can always delete without backtracking
 *
 * Deletion Cases:
 * Case 1: Key in leaf - simply remove
 * Case 2: Key in internal node
 *   2a: Left child has >= t keys -> replace with predecessor
 *   2b: Right child has >= t keys -> replace with successor
 *   2c: Both children have t-1 keys -> merge and recurse
 * Case 3: Key not in node (must descend)
 *   3a/3b: Borrow from sibling if possible
 *   3c: Merge with sibling if both have t-1 keys
 * ================================================================ */

/*
 * find_key - Find the index of a key in a node
 *
 * @node: node to search in
 * @key: key to find
 *
 * Returns: index i where keys[i] >= key, or n if key > all keys
 */
static int find_key(BTreeNode *node, int key) {
    int idx = 0;
    while (idx < node->n && node->keys[idx] < key) {
        idx++;
    }
    return idx;
}

/*
 * get_predecessor - Get the predecessor (largest key in left subtree)
 *
 * @node: internal node containing the key
 * @idx: index of the key whose predecessor we want
 *
 * The predecessor is the rightmost key in the left subtree,
 * i.e., follow children[idx] and then always go right.
 */
static int get_predecessor(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx];
    while (!cur->is_leaf) {
        cur = cur->children[cur->n];
    }
    return cur->keys[cur->n - 1];
}

/*
 * get_successor - Get the successor (smallest key in right subtree)
 *
 * @node: internal node containing the key
 * @idx: index of the key whose successor we want
 *
 * The successor is the leftmost key in the right subtree,
 * i.e., follow children[idx+1] and then always go left.
 */
static int get_successor(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx + 1];
    while (!cur->is_leaf) {
        cur = cur->children[0];
    }
    return cur->keys[0];
}

/*
 * merge - Merge children[idx] and children[idx+1] with keys[idx]
 *
 * @node: parent node
 * @idx: index of the key to pull down
 * @t: minimum degree
 *
 * Before merge (both children have t-1 keys):
 *     [..., K, ...]     <- parent, K = keys[idx]
 *        /     \
 *   [A B]       [C D]   <- children[idx] and children[idx+1]
 *
 * After merge:
 *     [...]             <- parent, K removed
 *       |
 *   [A B K C D]         <- merged child
 *
 * The merged child has 2t-1 keys (full but valid).
 */
static void merge(BTreeNode *node, int idx, int t) {
    BTreeNode *left = node->children[idx];
    BTreeNode *right = node->children[idx + 1];

    log_merge(node->keys[idx], idx);

    /* Pull down the key from parent into left child */
    left->keys[t - 1] = node->keys[idx];

    /* Copy all keys from right child to left child */
    for (int i = 0; i < right->n; i++) {
        left->keys[t + i] = right->keys[i];
    }

    /* Copy all children from right child to left child (if not leaf) */
    if (!left->is_leaf) {
        for (int i = 0; i <= right->n; i++) {
            left->children[t + i] = right->children[i];
        }
    }

    /* Update key count of left child */
    left->n = 2 * t - 1;

    /* Remove keys[idx] from parent by shifting */
    for (int i = idx; i < node->n - 1; i++) {
        node->keys[i] = node->keys[i + 1];
    }

    /* Remove children[idx+1] from parent by shifting */
    for (int i = idx + 1; i < node->n; i++) {
        node->children[i] = node->children[i + 1];
    }
    node->n--;

    /* Free the now-empty right child */
    free(right->keys);
    free(right->children);
    free(right);
}

/*
 * borrow_from_left - Borrow a key from left sibling through parent
 *
 * @node: parent node
 * @idx: index of the child that needs a key
 *
 * Before:
 *     [..., P, ...]          <- parent, P = keys[idx-1]
 *        /     \
 *   [A B C]     [D]          <- sibling has extra, child needs key
 *
 * After:
 *     [..., C, ...]          <- C moved up to parent
 *        /     \
 *   [A B]       [P D]        <- P moved down to child
 */
static void borrow_from_left(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx - 1];

    log_borrow_left(sibling->keys[sibling->n - 1], idx);

    /* Shift all keys in child to the right to make room at front */
    for (int i = child->n - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }

    /* Shift all children in child to the right (if not leaf) */
    if (!child->is_leaf) {
        for (int i = child->n; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
        /* Move sibling's rightmost child to child's first position */
        child->children[0] = sibling->children[sibling->n];
    }

    /* Move parent's key down to child's first position */
    child->keys[0] = node->keys[idx - 1];

    /* Move sibling's last key up to parent */
    node->keys[idx - 1] = sibling->keys[sibling->n - 1];

    child->n++;
    sibling->n--;
}

/*
 * borrow_from_right - Borrow a key from right sibling through parent
 *
 * @node: parent node
 * @idx: index of the child that needs a key
 *
 * Before:
 *     [..., P, ...]          <- parent, P = keys[idx]
 *        /     \
 *     [A]       [B C D]      <- child needs key, sibling has extra
 *
 * After:
 *     [..., B, ...]          <- B moved up to parent
 *        /     \
 *   [A P]       [C D]        <- P moved down to child
 */
static void borrow_from_right(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    log_borrow_right(sibling->keys[0], idx);

    /* Move parent's key down to child's last position */
    child->keys[child->n] = node->keys[idx];

    /* Move sibling's first child to child's last position (if not leaf) */
    if (!child->is_leaf) {
        child->children[child->n + 1] = sibling->children[0];
    }

    /* Move sibling's first key up to parent */
    node->keys[idx] = sibling->keys[0];

    /* Shift all keys in sibling to the left */
    for (int i = 0; i < sibling->n - 1; i++) {
        sibling->keys[i] = sibling->keys[i + 1];
    }

    /* Shift all children in sibling to the left (if not leaf) */
    if (!sibling->is_leaf) {
        for (int i = 0; i < sibling->n; i++) {
            sibling->children[i] = sibling->children[i + 1];
        }
    }

    child->n++;
    sibling->n--;
}

/*
 * fill - Ensure children[idx] has at least t keys
 *
 * @node: parent node
 * @idx: index of child that might need filling
 * @t: minimum degree
 *
 * Called before descending into children[idx] during delete.
 * If the child has only t-1 keys, we need to fill it:
 * - Try borrowing from left sibling first
 * - Try borrowing from right sibling
 * - If neither works, merge with a sibling
 */
static void fill(BTreeNode *node, int idx, int t) {
    /* Try borrowing from left sibling */
    if (idx > 0 && node->children[idx - 1]->n >= t) {
        borrow_from_left(node, idx);
    }
    /* Try borrowing from right sibling */
    else if (idx < node->n && node->children[idx + 1]->n >= t) {
        borrow_from_right(node, idx);
    }
    /* Must merge: prefer merging with left sibling */
    else {
        if (idx < node->n) {
            /* Merge with right sibling */
            merge(node, idx, t);
        } else {
            /* Merge with left sibling (idx is rightmost) */
            merge(node, idx - 1, t);
        }
    }
}

/*
 * delete_internal - Recursively delete a key from a subtree
 *
 * @node: root of subtree to delete from
 * @key: key to delete
 * @t: minimum degree
 *
 * This function handles all three cases of B-Tree deletion.
 */
static void delete_internal(BTreeNode *node, int key, int t) {
    int idx = find_key(node, key);

    /* Case 1 & 2: Key is in this node */
    if (idx < node->n && node->keys[idx] == key) {
        if (node->is_leaf) {
            /*
             * Case 1: Key is in a leaf node
             * Simply remove by shifting keys left
             */
            log_delete_leaf(key);
            for (int i = idx; i < node->n - 1; i++) {
                node->keys[i] = node->keys[i + 1];
            }
            node->n--;
        } else {
            /*
             * Case 2: Key is in an internal node
             */
            if (node->children[idx]->n >= t) {
                /*
                 * Case 2a: Left child has >= t keys
                 * Replace key with predecessor and delete predecessor
                 */
                int pred = get_predecessor(node, idx);
                log_delete_predecessor(key, pred);
                node->keys[idx] = pred;
                delete_internal(node->children[idx], pred, t);
            } else if (node->children[idx + 1]->n >= t) {
                /*
                 * Case 2b: Right child has >= t keys
                 * Replace key with successor and delete successor
                 */
                int succ = get_successor(node, idx);
                log_delete_successor(key, succ);
                node->keys[idx] = succ;
                delete_internal(node->children[idx + 1], succ, t);
            } else {
                /*
                 * Case 2c: Both children have t-1 keys
                 * Merge children, then delete key from merged child
                 */
                merge(node, idx, t);
                delete_internal(node->children[idx], key, t);
            }
        }
    } else {
        /*
         * Case 3: Key is not in this node
         * Must descend into appropriate child
         */
        if (node->is_leaf) {
            /* Key not found in tree */
            return;
        }

        /* Determine if we need to go into the last child */
        bool is_last_child = (idx == node->n);

        /*
         * Before descending, ensure the child has at least t keys
         * This is the proactive rebalancing step
         */
        if (node->children[idx]->n < t) {
            fill(node, idx, t);
        }

        /*
         * After fill(), the child index might have changed due to merge.
         * If we were going to the last child and a merge happened,
         * we need to go to the previous child now.
         */
        if (is_last_child && idx > node->n) {
            delete_internal(node->children[idx - 1], key, t);
        } else {
            delete_internal(node->children[idx], key, t);
        }
    }
}

/*
 * btree_delete - Delete a key from the B-Tree
 *
 * @tree: the B-Tree
 * @key: key to delete
 *
 * This is the public interface for deletion. It handles the special
 * case where the root becomes empty after deletion.
 */
void btree_delete(BTree *tree, int key) {
    if (!tree || !tree->root) return;
    if (tree->root->n == 0) return;  /* Empty tree */

    delete_internal(tree->root, key, tree->t);

    /*
     * Special case: if the root has no keys left but has a child,
     * make that child the new root. This is how tree height decreases.
     */
    if (tree->root->n == 0 && !tree->root->is_leaf) {
        BTreeNode *old_root = tree->root;
        tree->root = tree->root->children[0];
        free(old_root->keys);
        free(old_root->children);
        free(old_root);
    }
}

/* ================================================================
 * TRAVERSAL & DEBUG PRINTING
 * ================================================================ */

/*
 * btree_traverse - In-order traversal of B-Tree
 * 
 * For each node, we print keys in order:
 *   child[0], key[0], child[1], key[1], ..., child[n]
 */
void btree_traverse(BTreeNode *node) {
    if (!node) return;

    int i;
    for (i = 0; i < node->n; i++) {
        /* First, traverse left child of keys[i] */
        if (!node->is_leaf) {
            btree_traverse(node->children[i]);
        }
        printf("%d ", node->keys[i]);
    }

    /* Finally, traverse the rightmost child */
    if (!node->is_leaf) {
        btree_traverse(node->children[i]);
    }
}

/*
 * btree_print - Print tree using in-order traversal
 */
void btree_print(BTree *tree) {
    if (!tree || !tree->root || tree->root->n == 0) {
        printf("(empty tree)\n");
        return;
    }

    printf("B-Tree (t=%d): ", tree->t);
    btree_traverse(tree->root);
    printf("\n");
}

/* Helper for btree_print_debug */
static void print_node_debug(BTreeNode *node, int depth, int t) {
    if (!node) return;

    /* Print indentation */
    for (int i = 0; i < depth * 4; i++) {
        putchar(' ');
    }

    /* Print node info: [keys] (n=count, leaf/internal) */
    printf("[");
    for (int i = 0; i < node->n; i++) {
        printf("%d", node->keys[i]);
        if (i < node->n - 1) printf(" ");
    }
    printf("]");
    
    /* Print metadata */
    printf(" (n=%d, %s)", node->n, node->is_leaf ? "leaf" : "internal");
    
    /* Check for violations */
    if (depth > 0 && node->n < t - 1) {
        printf(" !! UNDERFLOW");
    }
    if (node->n > 2 * t - 1) {
        printf(" !! OVERFLOW");
    }
    printf("\n");

    /* Recursively print children */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            print_node_debug(node->children[i], depth + 1, t);
        }
    }
}

/*
 * btree_print_debug - Print tree structure with metadata
 */
void btree_print_debug(BTree *tree) {
    if (!tree || !tree->root) {
        printf("(null tree)\n");
        return;
    }

    printf("\n===== B-Tree Debug View (t=%d) =====\n", tree->t);
    print_node_debug(tree->root, 0, tree->t);
    printf("=====================================\n");
}

/* ================================================================
 * UTILITY FUNCTIONS (SKELETON)
 * ================================================================ */

/*
 * btree_height - Calculate the height of the tree
 * 
 * In a B-Tree, all leaves are at the same depth,
 * so we can just follow the leftmost path.
 */
int btree_height(BTree *tree) {
    if (!tree || !tree->root) return 0;
    if (tree->root->n == 0) return 0;

    int height = 1;
    BTreeNode *node = tree->root;
    
    while (!node->is_leaf) {
        height++;
        node = node->children[0];
    }
    
    return height;
}

/*
 * count_node - Recursively count keys in a subtree
 */
static int count_node(BTreeNode *node) {
    if (!node) return 0;

    int total = node->n;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            total += count_node(node->children[i]);
        }
    }
    return total;
}

/*
 * btree_count - Count total number of keys in tree
 */
int btree_count(BTree *tree) {
    if (!tree || !tree->root) return 0;
    return count_node(tree->root);
}

/*
 * validate_node - Recursively validate a node and its subtree
 *
 * @node: node to validate
 * @t: minimum degree
 * @min: minimum allowed key value (exclusive, INT_MIN for no limit)
 * @max: maximum allowed key value (exclusive, INT_MAX for no limit)
 * @expected_depth: expected depth of leaves (-1 to compute)
 * @current_depth: current depth in tree
 * @is_root: true if this is the root node
 *
 * Returns: depth of leaves if valid, -1 if invalid
 */
static int validate_node(BTreeNode *node, int t, int min, int max,
                         int expected_depth, int current_depth, bool is_root) {
    if (!node) return -1;

    /* Check 2: Key count bounds */
    if (is_root) {
        /* Root can have 1 to 2t-1 keys (or 0 if tree is empty) */
        if (node->n > 2 * t - 1) {
            fprintf(stderr, "Validation error: root has %d keys (max %d)\n",
                    node->n, 2 * t - 1);
            return -1;
        }
    } else {
        /* Non-root must have [t-1, 2t-1] keys */
        if (node->n < t - 1) {
            fprintf(stderr, "Validation error: node has %d keys (min %d)\n",
                    node->n, t - 1);
            return -1;
        }
        if (node->n > 2 * t - 1) {
            fprintf(stderr, "Validation error: node has %d keys (max %d)\n",
                    node->n, 2 * t - 1);
            return -1;
        }
    }

    /* Check 3: Keys are sorted and within range */
    for (int i = 0; i < node->n; i++) {
        if (node->keys[i] <= min || node->keys[i] >= max) {
            fprintf(stderr, "Validation error: key %d out of range (%d, %d)\n",
                    node->keys[i], min, max);
            return -1;
        }
        if (i > 0 && node->keys[i] <= node->keys[i - 1]) {
            fprintf(stderr, "Validation error: keys not sorted at index %d\n", i);
            return -1;
        }
    }

    /* Check 1: All leaves at same depth */
    if (node->is_leaf) {
        if (expected_depth != -1 && current_depth != expected_depth) {
            fprintf(stderr, "Validation error: leaf at depth %d, expected %d\n",
                    current_depth, expected_depth);
            return -1;
        }
        return current_depth;
    }

    /* Check 5: Internal node with k keys has k+1 children */
    /* Check 4: keys[i] properly separates children[i] and children[i+1] */
    int leaf_depth = expected_depth;
    for (int i = 0; i <= node->n; i++) {
        if (!node->children[i]) {
            fprintf(stderr, "Validation error: NULL child at index %d\n", i);
            return -1;
        }

        /* Determine the valid key range for this child */
        int child_min = (i == 0) ? min : node->keys[i - 1];
        int child_max = (i == node->n) ? max : node->keys[i];

        int depth = validate_node(node->children[i], t, child_min, child_max,
                                  leaf_depth, current_depth + 1, false);
        if (depth == -1) return -1;

        if (leaf_depth == -1) {
            leaf_depth = depth;  /* First leaf sets expected depth */
        }
    }

    return leaf_depth;
}

/*
 * btree_validate - Verify all B-Tree invariants
 *
 * Checks:
 * 1. All leaves at same depth
 * 2. Each node has [t-1, 2t-1] keys (root can have 1 to 2t-1)
 * 3. Keys in each node are sorted
 * 4. For internal nodes: keys[i] separates children[i] and children[i+1]
 * 5. Non-leaf with k keys has exactly k+1 children
 *
 * Returns: 1 if valid, 0 if invalid
 */
int btree_validate(BTree *tree) {
    if (!tree) return 0;
    if (!tree->root) return 0;

    /* Empty tree is valid */
    if (tree->root->n == 0 && tree->root->is_leaf) {
        return 1;
    }

    int result = validate_node(tree->root, tree->t, INT_MIN, INT_MAX,
                               -1, 0, true);
    return result != -1;
}
