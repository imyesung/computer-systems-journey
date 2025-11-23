# AVL Tree Demo: File and Function Overview

This directory implements a height-aware BST with AVL invariant checkers
and a small interactive demo driver.

## Current Status

- Tree mutations still call `insertBST` / `deleteBST`, so no rotations occur yet; `check_avl_invariant` will flag imbalances for many random inputs.
- The internal `rebalance` helper contains a detailed specification but currently just returns its input node unchanged.
- Placeholder `insertAVL` / `deleteAVL` functions live in `avl_tree.c` (not yet exported via the header) and simply return the tree unchanged after invoking the BST logic.

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
| `rebalance`           | Function    | `static`  | TODO stub: should update height + run LL/LR/RL/RR rotations |

### Public operations (exported API)

| Symbol         | Kind      | Description                                      |
|----------------|-----------|--------------------------------------------------|
| `searchBST`    | Function  | Recursive search, returns 0/1                    |
| `insertBST`    | Function  | BST insert + `height` maintenance                |
| `deleteBST`    | Function  | BST delete (0/1/2 children) + `height` update    |
| `freeTree`     | Function  | Postorder free of all nodes                      |

### AVL stubs (work in progress)

| Symbol         | Kind      | Description                                      |
|----------------|-----------|--------------------------------------------------|
| `insertAVL`    | Function  | Placeholder that will call BST insert then `rebalance` while unwinding; currently returns `root` unchanged |
| `deleteAVL`    | Function  | Placeholder mirroring `deleteBST` + `rebalance`; currently returns `root` unchanged |

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

1. Builds a random tree by calling `insertBST` 10 times.
2. Prints a sideways debug view via `print_tree_debug`.
3. Runs `check_avl_invariant` to verify structure and balance (the check will usually fail until rotations are implemented).
4. Asks the user for a key, deletes it with `deleteBST`, reprints the tree, and re-runs the invariant check.
5. Frees all memory with `freeTree` before exiting.

## TODO Checklist
1. Implement `rebalance`: recompute `height`, compute balance factor, and dispatch LL/LR/RL/RR rotations before returning the new subtree root.
2. Update insert logic: either expose `insertAVL` or upgrade `insertBST` so every recursive return path calls `rebalance`.
3. Update delete logic: mirror the insert plan so `deleteAVL` (or the upgraded `deleteBST`) rebalances on the way back up.
4. Once rotations are wired in, update `main.c` and the README again so the demo exercises the AVL-aware APIs.

