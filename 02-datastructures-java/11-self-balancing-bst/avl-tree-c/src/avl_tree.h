#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stdio.h>

/* Node structure */
struct Node {
    struct Node *left;
    struct Node *right;
    int key;
    int height;
};

/* ---------- Public Operations (API) ---------- */

/* Core Operations */
struct Node *insertBST(struct Node *node, int key);
struct Node *deleteBST(struct Node *root, int key);
int searchBST(struct Node *root, int key);
void freeTree(struct Node *root);

/* Debugging & Visualization */
void print_inorder(struct Node *root);
void print_tree_debug(struct Node *n, int depth);

/* Invariant Checkers */
int check_avl_invariant(struct Node *root);

#endif /* AVL_TREE_H */
