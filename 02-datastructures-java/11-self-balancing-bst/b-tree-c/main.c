#include <stdio.h>
#include "b-tree.h"

int main(void) {
    printf("===== B-Tree Demo =====\n\n");

    /* 
     * Create a B-Tree with minimum degree t=3
     * - Each node can have 2 to 5 keys
     * - Each internal node can have 3 to 6 children
     */
    BTree *tree = btree_create(3);
    if (!tree) {
        fprintf(stderr, "Failed to create B-Tree\n");
        return 1;
    }

    /* Test data for insertion */
    int keys[] = {10, 20, 5, 6, 12, 30, 7, 17, 3, 25, 35, 40, 15, 8, 1};
    int n = sizeof(keys) / sizeof(keys[0]);

    printf("Insertion order: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", keys[i]);
    }
    printf("\n\n");

    /* Insert keys one by one and show progress */
    for (int i = 0; i < n; i++) {
        printf("--- Inserting %d ---\n", keys[i]);
        btree_insert(tree, keys[i]);
        btree_print(tree);
        printf("\n");
    }

    /* Show final tree structure */
    printf("\n===== Final Tree =====\n");
    btree_print(tree);
    btree_print_debug(tree);
    printf("Height: %d\n", btree_height(tree));

    /* Cleanup */
    btree_destroy(tree);
    printf("\nTree destroyed. Memory freed.\n");

    return 0;
}
