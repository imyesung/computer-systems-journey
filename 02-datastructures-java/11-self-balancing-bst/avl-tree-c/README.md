# AVL Tree Demo: File and Function Overview

This directory implements a height-aware BST with AVL invariant checkers
and a small interactive demo driver.

## Current Status

- `insertAVL` / `deleteAVL` are fully wired: they wrap the BST logic and invoke `rebalance` on the way back up, so rotations apply immediately.
- `rebalance` now uses height-aware `rotate_left` / `rotate_right` helpers and emits optional trace logs showing LR/LL/RL/RR activity.
- The demo (`main.c`) first builds an imbalanced BST, then rebuilds the same keys with `insertAVL`, and finally lets you delete a key through `deleteAVL` to observe rebalancing in action.

## File Layout

| Path                                  | Role                                             |
|---------------------------------------|--------------------------------------------------|
| `src/avl_tree.h`                      | Public interface for the AVL/BST module         |
| `src/avl_tree.c`                      | Implementation of core operations and checkers  |
| `src/main.c`                          | Demo / driver: builds a tree and runs tests     |

## Build & Run

```bash
cd 02-datastructures-java/11-self-balancing-bst/avl-tree-c/src

gcc main.c avl_tree.c -std=c11 -Wall -Wextra -pedantic -o avl_demo
./avl_demo
```

## `src/avl_tree.h` (Public API)

| Symbol                   | Kind      | Description                                      |
|--------------------------|-----------|--------------------------------------------------|
| `struct Node`            | Type      | Tree node with `left`, `right`, `key`, `height` |
| `insertBST`              | Function  | Insert a key into a height-aware BST            |
| `deleteBST`              | Function  | Delete a key from the height-aware BST          |
| `searchBST`              | Function  | Return non-zero if `key` exists in the tree     |
| `freeTree`               | Function  | Recursively free all nodes in the tree          |
| `print_inorder`          | Function  | Inorder traversal, prints keys in sorted order  |
| `print_tree_debug`       | Function  | Sideways tree print with height / balance info  |
| `check_avl_invariant`    | Function  | Verify BST order and AVL height/balance rules   |

## `src/avl_tree.c` (Implementation)

### Core data and helpers (internal)

| Symbol                | Kind        | Visibility | Description                                      |
|-----------------------|------------|-----------|--------------------------------------------------|
| `NIL_HEIGHT`          | Macro       | Internal  | Height of an empty subtree (`-1`)               |
| `newNode`             | Function    | `static`  | Allocate and initialize a new leaf node         |
| `getHeight`           | Function    | `static`  | Safely read `height` (`NIL_HEIGHT` for `NULL`)  |
| `max`                 | Function    | `static`  | Utility to compute max of two integers          |
| `updateHeight`        | Function    | `static`  | Recompute `height` from children                |
| `getBalanceFactor`    | Function    | `static`  | `left_height - right_height` for a node         |
| `minValueNode`        | Function    | `static`  | Leftmost node in a subtree (used by delete)     |
| `rotate_left/right`   | Function    | `static`  | Perform rotations and refresh heights locally   |
| `rebalance`           | Function    | `static`  | Updates height, checks BF, and dispatches LL/LR/RL/RR rotations with optional tracing |

### Public operations (exported API)

| Symbol         | Kind      | Description                                      |
|----------------|-----------|--------------------------------------------------|
| `searchBST`    | Function  | Recursive search, returns 0/1                    |
| `insertBST`    | Function  | BST insert + `height` maintenance                |
| `deleteBST`    | Function  | BST delete (0/1/2 children) + `height` update    |
| `freeTree`     | Function  | Postorder free of all nodes                      |

### AVL-aware operations

| Symbol         | Kind      | Description                                      |
|----------------|-----------|--------------------------------------------------|
| `insertAVL`    | Function  | BST insert + `rebalance` while unwinding; exported in the public header |
| `deleteAVL`    | Function  | BST delete + `rebalance` while unwinding; exported in the public header |

### Traversal and debug printing

| Symbol             | Kind      | Description                                      |
|--------------------|-----------|--------------------------------------------------|
| `print_inorder`    | Function  | Inorder traversal, prints keys                   |
| `print_tree_debug` | Function  | Sideways tree view + `(h, bf)` and `!!` marks    |
| `INDENT_STEP`      | Macro     | Horizontal indent per depth level                |
| `COMMENT_COL`      | Macro     | Column where debug comment block starts          |

### Invariant checkers

| Symbol               | Kind      | Visibility | Description                                      |
|----------------------|-----------|-----------|--------------------------------------------------|
| `check_range`        | Function  | `static`  | Helper: checks BST order by min/max range        |
| `check_bst_invariant`| Function  | `static`  | Verify pure BST ordering property                |
| `check_avl_subtree`  | Function  | `static`  | Recursive check of height + balance for subtree  |
| `check_avl_invariant`| Function  | Public    | Full check: AVL balance + height + BST order     |


## `src/main.c` (Demo / Driver)

| Symbol        | Kind      | Description                                      |
|---------------|-----------|--------------------------------------------------|
| `random_key`  | Function  | Generate a random key in `[0, max_value)`       |
| `main`        | Function  | Demo: build tree, show debug view, delete key   |

The `main` function:

1. Builds a random tree with `insertBST` to showcase an imbalanced starting point.
2. Prints a sideways debug view + automated invariant check (usually FAIL at this stage).
3. Rebuilds the same keys with `insertAVL`, printing rotation traces (if enabled) and verifying the AVL invariants now PASS.
4. Prompts for a key, deletes it via `deleteAVL`, and shows the balanced tree plus the invariant check.
5. Frees all memory with `freeTree` before exiting.

