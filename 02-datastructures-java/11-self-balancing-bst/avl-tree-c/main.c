#include <stdio.h>     // for printf
#include <stdlib.h>    // for malloc, free

#define NIL_HEIGHT (-1)

/*  rand() function to generate a random key to focus on AVL tree balancing */
int random_key(int max_value) {
    if (max_value <= 0) return 0;
    return rand() % max_value;
}

/* Node structure */
struct Node {
    struct Node* left;
    struct Node* right;
    int key;
    int height;
};

/* newNode */
struct Node *newNode(int key) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 0;
    return node;
}

/* insertBST */
struct Node* insertBST(struct Node* node, int key) {
    if (node == NULL) {
        return newNode(key);
    }
    if (key < node->key) {
        node->left = insertBST(node->left, key);
    } else if (key > node->key) {
        node->right = insertBST(node->right, key);
    } else {
        return node;
    }
    return node;
}

/* print_inorder: Inorder traversal prints BST keys in ascending order. */
void print_inorder(struct Node* root) {
    if (root == NULL) return;

    print_inorder(root->left);
    printf("%d ", root->key);
    print_inorder(root->right);
}


/* print_tree: Visual representation of the tree structure. */
static void print_tree(struct Node *n, int depth)
{
    int i;
    if (!n) return;
    
    for (i = 0; i < depth; i++)
        printf("  ");
    printf("%d\n", n->key);
    
    print_tree(n->right, depth + 1);
    print_tree(n->left, depth + 1);
}

/* freeTree: Postorder traversal frees children before the parent to avoid dangling pointers. */
void freeTree(struct Node* root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main(void) {
    struct Node* root = NULL;

    // Insert random keys into the BST
    for (int i = 0; i < 10; i++) {
        int key = random_key(1000);
        root = insertBST(root, key);
    }

    printf("Tree:\n");
    print_tree(root, 0);
    
    printf("Inorder traversal:\n");
    print_inorder(root);
    printf("\n");

    freeTree(root);

    return 0;
}
