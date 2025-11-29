#ifndef SPLAY_TREE_H
#define SPLAY_TREE_H

struct Node {
    int key;
    struct Node *left;
    struct Node *right;
};

/* Public Splay Tree API (to be implemented incrementally) */
struct Node *splay_search(struct Node *root, int key);
struct Node *splay_insert(struct Node *root, int key);
struct Node *splay_delete(struct Node *root, int key);

/* Utility helpers */
void print_inorder(struct Node *root);
void print_tree_debug(struct Node *root, int depth);
void freeTree(struct Node *root);

#endif /* SPLAY_TREE_H */
