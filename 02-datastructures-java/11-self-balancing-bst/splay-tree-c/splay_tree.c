#include "splay_tree.h"
#include <stdlib.h>
#include <stdio.h>

/* Helper: create a new node */
static struct Node *createNode(int key) {
    struct Node *node = malloc(sizeof(struct Node));
    if (node) {
        node->key = key;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

/* ------------------ Rotations ------------------ */
// Right Rotation
//     y                x
//    / \              / \
//   x   C    --->    A   y
//  / \                  / \
// A   B                B   C
static struct Node *rotateRight(struct Node *y) {
    struct Node *x = y->left;
    struct Node *B = x->right;
    
    x->right = y;
    y->left = B;
    
    return x;  // x -> new root
}

// Left Rotation
//   x                  y
//  / \                / \
// A   y     --->     x   C
//    / \            / \
//   B   C          A   B
static struct Node *rotateLeft(struct Node *x) {
    struct Node *y = x->right;
    struct Node *B = y->left;
    
    y->left = x;
    x->right = B;
    
    return y;  // y -> new root
}

/* ------------------ Splay Tree Operations ------------------ */
static struct Node *splay(struct Node *root, int key) {
    if (root == NULL || root->key == key) {
        return root;
    }

    /* key < root->key case */
    if (key < root->key) {
        if (root->left == NULL) return root;

        // Zig-Zig (Left Left)
        if (key < root->left->key) {
            root->left->left = splay(root->left->left, key);
            root = rotateRight(root);
        }
        // Zig-Zag (Left Right)
        else if (key > root->left->key) {
            root->left->right = splay(root->left->right, key);
            if (root->left->right)
                root->left = rotateLeft(root->left);
        }
        // Zig
        return (root->left == NULL) ? root : rotateRight(root);
    }
    /* key < root->key case */
    else {
        if (root->right == NULL) return root;

        // Zig-Zig (Right Right)
        if (key > root->right->key) {
            root->right->right = splay(root->right->right, key);
            root = rotateLeft(root);
        }
        // Zig-Zag (Right Left)
        else if (key < root->right->key) {
            root->right->left = splay(root->right->left, key);
            if (root->right->left)
                root->right = rotateRight(root->right);
        }
        // Zig
        return (root->right == NULL) ? root : rotateLeft(root);
    }
}

/* BST Insert with splay (new key ends up at root) */
struct Node *splay_insert(struct Node *root, int key) {
    if (root == NULL) {
        return createNode(key);
    }

    /* Splay the closest node to the top -> attach in O(1) */
    root = splay(root, key);
    if (root->key == key) {
        return root; // duplicate: no-op
    }

    struct Node *node = createNode(key);
    if (node == NULL) {
        return root; // allocation failed, keep old tree
    }

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

// BST Search that splays the accessed node to root
struct Node *splay_search(struct Node *root, int key) {
    return splay(root, key);
}

// BST Delete with splay
struct Node *splay_delete(struct Node *root, int key) {
    if (root == NULL) {
        return NULL;
    }

    /* Bring the target (or the closest node) to the root */
    root = splay(root, key);
    if (root->key != key) {
        return root;
    }

    if (root->left == NULL) {
        struct Node *temp = root->right;
        free(root);
        return temp;
    }
    
    /* Splay the maximum node of the left subtree to the top.
    * We splay with the original root key, so the search walks
    * down the right spine and stops at max(left). */
    struct Node *rightSub = root->right;
    struct Node *newRoot = splay(root->left, key); // key is > any in left subtree
    newRoot->right = rightSub;

    free(root);
    return newRoot;
}

/* ------------------ Utility Helpers ------------------ */
void print_inorder(struct Node *root) {
    if (root == NULL) {
        return;
    }
    print_inorder(root->left);
    printf("%d ", root->key);
    print_inorder(root->right);
}

void print_tree_debug(struct Node *root, int depth) {
    if (root == NULL) {
        return;
    }

    print_tree_debug(root->right, depth + 1);

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%d\n", root->key);

    print_tree_debug(root->left, depth + 1);
}

void freeTree(struct Node *root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}
