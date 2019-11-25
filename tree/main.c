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

    TreeResize(tree, 30);
    TreeResize(tree, 15);

    TreeFree(tree);

    return 0;
}