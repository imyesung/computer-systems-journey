/*
 * B-Tree Implementation
 * 
 * This file implements the core B-Tree operations.
 * See b-tree.h for the study checklist and B-Tree properties.
 */

#include "b-tree.h"
#include <stdio.h>
#include <string.h>

/* Enable/disable trace logging for debugging */
#define TRACE_SPLIT 1

/* ================================================================
 * INTERNAL HELPER FUNCTIONS (Static)
 * ================================================================ */

static BTreeNode *create_node(int t, bool is_leaf);
static void destroy_node(BTreeNode *node);
static void split_child(BTreeNode *parent, int i, int t);
static void insert_non_full(BTreeNode *node, int key, int t);

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
 * 
 * TODO: Implement this function
 */
BTreeNode *btree_search(BTreeNode *node, int key, int *idx) {
    /* TODO: Implement search
     * 
     * int i = 0;
     * while (i < node->n && key > node->keys[i]) {
     *     i++;
     * }
     * 
     * if (i < node->n && key == node->keys[i]) {
     *     *idx = i;
     *     return node;
     * }
     * 
     * if (node->is_leaf) {
     *     return NULL;
     * }
     * 
     * return btree_search(node->children[i], key, idx);
     */
    return NULL;
}

/* ================================================================
 * DELETE OPERATION (SKELETON)
 * ================================================================ */

/*
 * btree_delete - Delete a key from the B-Tree
 * 
 * @tree: the B-Tree
 * @key: key to delete
 * 
 * B-Tree deletion is complex with multiple cases:
 * 
 * Case 1: Key is in a LEAF node
 *   - Simply remove the key
 *   - May cause underflow (n < t-1), handled by parent
 * 
 * Case 2: Key is in an INTERNAL node
 *   2a: If left child has >= t keys, replace with predecessor
 *   2b: If right child has >= t keys, replace with successor
 *   2c: If both children have t-1 keys, merge them
 * 
 * Case 3: Key is NOT in current node (descending)
 *   3a: If child has only t-1 keys but left sibling has >= t, borrow
 *   3b: If child has only t-1 keys but right sibling has >= t, borrow
 *   3c: If both siblings have t-1 keys, merge with one sibling
 * 
 * TODO: Implement helper functions and main delete logic
 */
void btree_delete(BTree *tree, int key) {
    /* TODO: Implement delete
     * 
     * Helper functions needed:
     * - find_key(): locate key position in node
     * - get_predecessor(): find largest key in left subtree
     * - get_successor(): find smallest key in right subtree  
     * - merge(): merge two children with parent key
     * - borrow_from_left(): rotate key from left sibling
     * - borrow_from_right(): rotate key from right sibling
     * - delete_from_leaf(): remove key from leaf
     * - delete_from_internal(): handle internal node deletion
     * - fill(): ensure child has at least t keys before descending
     */
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
 * btree_count - Count total number of keys in tree
 * 
 * TODO: Implement using recursive traversal
 */
int btree_count(BTree *tree) {
    /* TODO: Implement count
     * 
     * static int count_node(BTreeNode *node) {
     *     if (!node) return 0;
     *     int total = node->n;
     *     if (!node->is_leaf) {
     *         for (int i = 0; i <= node->n; i++) {
     *             total += count_node(node->children[i]);
     *         }
     *     }
     *     return total;
     * }
     */
    return 0;
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
 * TODO: Implement validation
 * 
 * Returns: 1 if valid, 0 if invalid
 */
int btree_validate(BTree *tree) {
    /* TODO: Implement validation */
    return 1;
}
