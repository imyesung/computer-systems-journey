/* ============================================================
 * B-Tree Properties (Minimum Degree t >= 2)
 * ============================================================
 * - Every node has at most 2t-1 keys
 * - Every node (except root) has at least t-1 keys
 * - Root has at least 1 key (if tree is non-empty)
 * - All leaves are at the same depth
 * - A non-leaf with k keys has k+1 children
 * 
 * ============================================================
 * STUDY CHECKLIST - Essential B-Tree Operations to Understand
 * ============================================================
 * 
 * [x] 1. Node Structure
 *     - keys[]: array of keys (max 2t-1)
 *     - children[]: array of child pointers (max 2t)
 *     - n: current number of keys
 *     - is_leaf: boolean flag
 * 
 * [x] 2. CREATE / DESTROY
 *     - btree_create(): allocate tree and empty root
 *     - btree_destroy(): recursively free all nodes
 * 
 * [x] 3. INSERT (Proactive Splitting)
 *     - split_child(): split a full child before descending
 *     - insert_non_full(): insert into a node that has room
 *     - btree_insert(): handle root split specially
 *
 * [x] 4. SEARCH
 *     - btree_search(): find key in tree, return node and index
 *
 * [x] 5. DELETE (Complex - Multiple Cases)
 *     - Case 1: Key in leaf node
 *     - Case 2: Key in internal node
 *       - 2a: Replace with predecessor
 *       - 2b: Replace with successor
 *       - 2c: Merge children
 *     - Case 3: Key not in current node
 *       - 3a: Borrow from left sibling
 *       - 3b: Borrow from right sibling
 *       - 3c: Merge with sibling
 *
 * [x] 6. TRAVERSAL
 *     - btree_traverse(): in-order traversal
 *     - btree_print_debug(): visual tree structure
 *
 * [x] 7. UTILITY
 *     - btree_height(): tree height
 *     - btree_count(): total key count
 *     - btree_validate(): check B-Tree invariants
 * ============================================================ */

#ifndef B_TREE_H
#define B_TREE_H

#include <stdbool.h>
#include <stdlib.h>

/* ---------- Data Structures ---------- */

typedef struct BTreeNode {
    int *keys;                    /* Array of keys (max 2t-1) */
    struct BTreeNode **children;  /* Array of child pointers (max 2t) */
    int n;                        /* Current number of keys */
    bool is_leaf;                 /* True if this is a leaf node */
} BTreeNode;

typedef struct BTree {
    BTreeNode *root;
    int t;  /* Minimum degree: each node has [t-1, 2t-1] keys */
} BTree;

/* ---------- Create / Destroy ---------- */

BTree *btree_create(int t);
void btree_destroy(BTree *tree);

/* ---------- Core Operations ---------- */

void btree_insert(BTree *tree, int key);
BTreeNode *btree_search(BTreeNode *node, int key, int *idx);
void btree_delete(BTree *tree, int key);

/* ---------- Traversal ---------- */

void btree_traverse(BTreeNode *node);
void btree_print(BTree *tree);
void btree_print_debug(BTree *tree);

/* ---------- Utility ---------- */

int btree_height(BTree *tree);
int btree_count(BTree *tree);
int btree_validate(BTree *tree);

#endif /* B_TREE_H */
