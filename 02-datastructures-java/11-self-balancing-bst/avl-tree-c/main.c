#include <stdio.h>     // for printf, scanf, fprintf, stderr
#include <stdlib.h>    // for malloc, free, rand, exit, abs

#define NIL_HEIGHT (-1)

/*  rand() function to generate a random key to focus on AVL tree balancing  */
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
    node->height = 0;  // leaf height is considered 0
    return node;
}

/* ---------- AVL Helpers (prevent segfaults and encapsulate complexity) ---------- */
/* Safe accessor for height. Returns NIL_HEIGHT (-1) for NULL nodes. */
int getHeight(struct Node *n) {
    if (n == NULL) return NIL_HEIGHT;
    return n->height;
}

/* Utility to get maximum of two integers */
int max(int a, int b) {
    return (a > b) ? a : b;
}

/* updateHeight: the height of the node based on children.
 * This must be called after any modification (insert/delete) to the children.
 */
void updateHeight(struct Node *n) {
    if (n != NULL) {
        n->height = 1 + max(getHeight(n->left), getHeight(n->right));
    }
}

/* getBalanceFactor: Calculates Balance Factor (Left Height - Right Height).
 * Positive: Left heavy / Negative: Right heavy
 * Ideal range: -1, 0, 1. If outside this range, rotation is needed.
 */
int getBalanceFactor(struct Node *n) {
    if (n == NULL) return 0;
    return getHeight(n->left) - getHeight(n->right);
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

/* insertBST: Recursive insertion with Height Update */
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

    /* Update Height */
    // As the recursion unwinds, update current ancestor node heights.
    updateHeight(node);

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

/* deleteBST: Recursive deletion with Height Update */
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
            return NULL; // No node remaining, no height to update
        }

        // Case B: exactly one child
        if (root->left == NULL || root->right == NULL) {
            struct Node *child = (root->left != NULL) ? root->left : root->right;
            free(root);
            return child; // Child takes place, parent will update its height
        }

        // Case C: two children
        struct Node *successor = minValueNode(root->right);
        root->key = successor->key;
        root->right = deleteBST(root->right, successor->key);
    }

    /* Update Height */
    // If the node itself was deleted (returning NULL or child above),
    // this line updates the *current* node in the recursion stack (the parent of deleted one).
    if (root != NULL) {
        updateHeight(root);
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

/* print_tree_debug: Shows structure + Height + Balance Factor.
 * Marks unbalanced nodes with "!!" to indicate where rotation is needed.
 */
#define INDENT_STEP 4
#define COMMENT_COL 32

static void print_tree_debug(struct Node *n, int depth) {
    if (n == NULL) return;

    // Print Right side
    print_tree_debug(n->right, depth + 1);

    for (int i = 0; i < depth * INDENT_STEP; i++) {
        putchar(' ');
    }

    printf("%4d", n->key);

    // Compute current column and pad up to COMMENT_COL
    int current_col = depth * INDENT_STEP + 4;   // indent + key width
    int spaces = COMMENT_COL - current_col;
    if (spaces < 1) spaces = 1;  // at least 1 space

    for (int i = 0; i < spaces; i++) {
        putchar(' ');
    }

    // Print metadata as a "comment column"
    int h  = getHeight(n);
    int bf = getBalanceFactor(n);

    if (abs(bf) > 1) {
        printf("(h=%2d, bf=%+2d)  !!\n", h, bf);
    } else {
        printf("(h=%2d, bf=%+2d)\n", h, bf);
    }

    // Print Left side
    print_tree_debug(n->left, depth + 1);
}

/* freeTree: postorder traversal frees children before the parent */
void freeTree(struct Node *root) {
    if (root == NULL) {   // empty subtree is always valid.
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
    if (n == NULL) return 1;

    if ((has_min && n->key <= min_ok) ||
        (has_max && n->key >= max_ok)) {
        return 0;
    }

    return check_range(n->left,  min_ok,     n->key, has_min, 1) &&
           check_range(n->right, n->key, max_ok,     1,       has_max);
}

int check_bst_invariant(struct Node *root) {
    return check_range(root, 0, 0, 0, 0);
}

/* ---------- main: simple interactive demo ---------- */

int main(void) {
    struct Node *root = NULL;

    printf("--- Generating Random Tree ---\n");
    for (int i = 0; i < 10; i++) {
        int key = random_key(1000);
        root = insertBST(root, key);
    }

    printf("Initial tree (BST invariant: %s)\n",
           check_bst_invariant(root) ? "OK" : "VIOLATED");
    
    printf("\n[Visual Debug Dashboard]\n");
    printf("Check for nodes marked with '!!' (Unbalanced)\n");
    printf("--------------------------------------------------\n");
    print_tree_debug(root, 0);
    printf("--------------------------------------------------\n");

    printf("Inorder traversal: ");
    print_inorder(root);
    printf("\n\n");

    /* Delete Operation Test */
    printf("Enter a key to delete: ");
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
        
        printf("\n[Visual Debug Dashboard (After Delete)]\n");
        print_tree_debug(root, 0);
        
        printf("Inorder traversal: ");
        print_inorder(root);
        printf("\n");
    }

    freeTree(root);
    return 0;
}
