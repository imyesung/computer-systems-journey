#include <stdio.h>
#include <stdlib.h>
#include "avl_tree.h"

/* rand() function to generate a random key */
int random_key(int max_value) {
    if (max_value <= 0) return 0;
    return rand() % max_value;
}

int main(void) {
    struct Node *root = NULL;

    printf("--- Generating Random Tree ---\n");
    for (int i = 0; i < 10; i++) {
        int key = random_key(1000);
        root = insertBST(root, key);
    }

    printf("\n[Visual Dashboard]\n");
    printf("Nodes flagged with '!!' need rebalancing\n");
    printf("--------------------------------------------------\n");
    print_tree_debug(root, 0);
    printf("--------------------------------------------------\n");

    printf("\n[Automated Verification]\n");
    if (check_avl_invariant(root)) {
        printf("RESULT: PASS (Valid AVL Tree)\n");
    } else {
        printf("RESULT: FAIL (Invariant Violated. Rotation needed.)\n");
    }

    printf("\nInorder traversal: ");
    print_inorder(root);
    printf("\n\n");

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
        root = deleteBST(root, target);
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
