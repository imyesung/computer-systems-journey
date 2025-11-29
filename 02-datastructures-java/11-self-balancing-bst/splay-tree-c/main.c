#include <stdio.h>
#include <assert.h>
#include "splay_tree.h"

int main(void) {
    struct Node *root = NULL;

    int keys[] = {10, 20, 5, 15, 30};
    int n = sizeof(keys) / sizeof(keys[0]);

    /* Quick smoke test for splay operations. */
    root = splay_insert(root, 10);
    assert(root && root->key == 10);

    root = splay_insert(root, 5);
    root = splay_insert(root, 15);

    root = splay_search(root, 5);
    assert(root && root->key == 5);   /* search should splay 5 to root */

    root = splay_delete(root, 10);
    assert(root && root->key != 10);  /* 10 should be removed */

    freeTree(root);
    root = NULL;

    for (int i = 0; i < n; i++) {
        root = splay_insert(root, keys[i]);
        printf("After insert %d:\n", keys[i]);
        print_tree_debug(root, 0);
        putchar('\n');
    }

    root = splay_search(root, 15);
    printf("After search 15 (splayed to root):\n");
    print_tree_debug(root, 0);
    putchar('\n');

    root = splay_delete(root, 20);
    printf("After delete 20:\n");
    print_tree_debug(root, 0);
    putchar('\n');

    printf("Inorder: ");
    print_inorder(root);
    putchar('\n');

    freeTree(root);
    return 0;
}
