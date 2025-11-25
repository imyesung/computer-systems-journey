#include <stdio.h>
#include <stdlib.h>
#include "avl_tree.h"

#define SAMPLE_INSERTS 10

static void wait_for_enter(const char *prompt) {
    int ch = 0;
    printf("%s", prompt);
    fflush(stdout);
    while ((ch = getchar()) != '\n' && ch != EOF) {
        // discard extra input until newline
    }
}

/* rand() function to generate a random key */
int random_key(int max_value) {
    if (max_value <= 0) return 0;
    return rand() % max_value;
}

int main(void) {
    struct Node *root = NULL;
    int keys[SAMPLE_INSERTS];

    printf("--- Generating Random Tree (unbalanced BST inserts) ---\n");
    for (int i = 0; i < SAMPLE_INSERTS; i++) {
        int key = random_key(1000);
        keys[i] = key;
        root = insertBST(root, key);
    }

    printf("\n[Visual Dashboard]\n");
    printf("Nodes flagged with '!!' need rebalancing\n");
    printf("--------------------------------------------------\n");
    print_tree_debug(root, 0);
    printf("--------------------------------------------------\n");

    printf("\n[Automated Verification]\n");
    int initial_valid = check_avl_invariant(root);
    if (initial_valid) {
        printf("RESULT: PASS (Valid AVL Tree)\n");
    } else {
        printf("RESULT: FAIL (Invariant Violated. Rotation needed.)\n");
        printf("Use the diagnostics above to see every node that broke AVL rules.\n");
        wait_for_enter("Press Enter to rebuild the same keys with AVL insertions (or Ctrl+C to inspect manually)...\n");
    }

    printf("\nInorder traversal: ");
    print_inorder(root);
    printf("\n\n");

    printf("--- Rebuilding Using insertAVL (same keys) ---\n");
    struct Node *avl_root = NULL;
    for (int i = 0; i < SAMPLE_INSERTS; i++) {
        avl_root = insertAVL(avl_root, keys[i]);
    }

    printf("\n[Visual Dashboard After Rebuild]\n");
    print_tree_debug(avl_root, 0);
    printf("--------------------------------------------------\n");

    printf("\n[Automated Verification After Rebuild]\n");
    if (check_avl_invariant(avl_root)) {
        printf("RESULT: PASS (Valid AVL Tree)\n");
    } else {
        printf("RESULT: FAIL (Unexpected imbalance)\n");
    }

    printf("\nInorder traversal: ");
    print_inorder(avl_root);
    printf("\n\n");

    freeTree(root);
    root = avl_root;

    printf("[Delete Operation Test]\n");
    printf("Enter a key to delete: ");
    int target = 0;
    if (scanf("%d", &target) != 1) {
        fprintf(stderr, "Invalid input\n");
        freeTree(root);
        return 1;
    }

    if (!searchBST(root, target)) {
        printf("\nKey %d not found. Tree unchanged.\n", target);
    } else {
        root = deleteAVL(root, target);
        printf("\n[Visual Dashboard After Delete]\n");
        print_tree_debug(root, 0);
        
        printf("\n[Automated Verification After Delete]\n");
        if (check_avl_invariant(root)) {
            printf("RESULT: PASS (Valid AVL Tree)\n");
        } else {
            printf("RESULT: FAIL (Invariant Violated. Rotation needed.)\n");
        }

        printf("\nInorder traversal: ");
        print_inorder(root);
        printf("\n");
    }

    freeTree(root);
    return 0;
}
