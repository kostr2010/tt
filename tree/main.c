#include <stdio.h>
#include <stdlib.h>

#include "tree.h"

//####################//

#define SEC_ON
#define LOG_ON

//####################//

int main () {
    Tree* tree = TreeAlloc();
    TREE_INIT(tree, 2);
    TreeSort(tree);

    TreeAddNode(tree, tree->root, left, 100);
    TreeAddNode(tree, tree->root, left, 99);
    TreeAddNode(tree, tree->root, mid, 98);
    TreeAddNode(tree, TreeFind(tree, tree->root, 98), left, 97);
    TreeAddNode(tree, TreeFind(tree, tree->root, 97), left, 96);
    for (int i = 96; i > 60; i--) {
        printf("addr of %d: %d\n", i, TreeFind(tree, tree->root, i));
        TreeAddNode(tree, TreeFind(tree, tree->root, i), mid, i - 1);
    }
    TreeDeleteNode(tree, TreeFind(tree, tree->root, 98));

    TreeFree(tree);

    return 0;
}