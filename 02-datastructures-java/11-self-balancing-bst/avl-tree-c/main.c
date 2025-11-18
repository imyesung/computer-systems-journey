#include <stdio.h>     // for printf, scanf, fprintf, stderr
#include <stdlib.h>    // for malloc, free, rand, exit

#define NIL_HEIGHT (-1)

/*  rand() function to generate a random key to focus on AVL tree balancing */
int random_key(int max_value) {
    if (max_value <= 0) return 0;
    return rand() % max_value;
}

/* Node structure */
struct Node {
    struct Node *left;
    struct Node *right;
    int key;
    int height;
};

/* ---------- Node creation ---------- */

struct Node *newNode(int key) {
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 0;  // leaf height; consistent with NIL_HEIGHT = -1
    return node;
}

/* ---------- Basic BST operations (search / insert / delete) ---------- */

/* containsKey: check if a key exists in the BST */
static int containsKey(struct Node *root, int key) {
    if (root == NULL) return 0;
    if (key == root->key) return 1;
    return (key < root->key)
        ? containsKey(root->left, key)
        : containsKey(root->right, key);
}

/* insertBST: standard recursive BST insertion, no duplicates */
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
    return node;
}

/* minValueNode: return the leftmost (smallest-key) node in this subtree */
static struct Node *minValueNode(struct Node *node) {
    struct Node *current = node;
    while (current != NULL && current->left != NULL) {
        current = current->left;
    }
    return current;
}

/* deleteBST: remove a node with the given key from the BST */
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
        // Case A: no children
        if (root->left == NULL && root->right == NULL) {
            free(root);
            return NULL;
        }

        // Case B: exactly one child
        if (root->left == NULL || root->right == NULL) {
            struct Node *child = (root->left != NULL) ? root->left : root->right;
            free(root);
            return child;
        }

        // Case C: two children
        struct Node *successor = minValueNode(root->right);
        /* successor = smallest key in right subtree (inorder next). 
           Replace current key with successor’s key, then delete the original successor node. */
        root->key = successor->key;
        root->right = deleteBST(root->right, successor->key);
    }

    return root;
}

/* ---------- Traversal / printing / memory ---------- */

/* print_inorder: inorder traversal prints BST keys in ascending order */
void print_inorder(struct Node *root) {
    if (root == NULL) return;

    print_inorder(root->left);
    printf("%d ", root->key);
    print_inorder(root->right);
}

/* print_tree: visual representation of the tree structure */
static void print_tree(struct Node *n, int depth) {
    if (n == NULL) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("%d\n", n->key);

    print_tree(n->right, depth + 1);
    print_tree(n->left, depth + 1);
}

/* freeTree: postorder traversal frees children before the parent */
void freeTree(struct Node *root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

/* ---------- BST invariant checker ---------- */

static int check_range(struct Node *n,
                       int min_ok, int max_ok,
                       int has_min, int has_max) {
    if (n == NULL) return 1;  /* empty subtree is always valid */

    if ((has_min && n->key <= min_ok) ||
        (has_max && n->key >= max_ok)) {
        return 0;
    }

    // left child:  (min_ok, n->key)  if has_min;
    // right child: (n->key, max_ok)  if has_max; strictly inside the range
    return check_range(n->left,  min_ok,     n->key, has_min, 1) &&
           check_range(n->right, n->key, max_ok,     1,       has_max);
}

int check_bst_invariant(struct Node *root) {
    return check_range(root, 0, 0, 0, 0);
}

/* ---------- main: simple interactive demo ---------- */

int main(void) {
    struct Node *root = NULL;

    for (int i = 0; i < 10; i++) {
        int key = random_key(1000);
        root = insertBST(root, key);
    }

    printf("Initial tree (BST invariant: %s)\n",
           check_bst_invariant(root) ? "OK" : "VIOLATED");
    print_tree(root, 0);

    printf("Inorder traversal:\n");
    print_inorder(root);
    printf("\n\nEnter a key to delete: ");

    int target = 0;
    if (scanf("%d", &target) != 1) {
        fprintf(stderr, "Invalid input\n");
        freeTree(root);
        return 1;
    }

    if (!containsKey(root, target)) {
        printf("\nKey %d not found. Tree unchanged.\n", target);
    } else {
        root = deleteBST(root, target);
        printf("\nAfter deletion (BST invariant: %s)\n",
               check_bst_invariant(root) ? "OK" : "VIOLATED");
        print_tree(root, 0);
        printf("Inorder traversal:\n");
        print_inorder(root);
        printf("\n");
    }

    freeTree(root);
    return 0;
}
