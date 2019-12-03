#include <stdio.h>

#include "tree.h"

//####################//


//####################//

int main() {
    Tree* tree = TreeAlloc();
    if (TreeInit(tree, "ass") == -1)
        printf("no init for you!\n");

    if (TreeDeleteNode(tree, TreeGetRoot(tree)) == -1)
        printf("no delete for you!\n");
    
    if (TreeInsertNode(tree, TreeGetRoot(tree), left, 13) == -1)
        printf("no insert for you!\n");

    if (TreeInsertNode(tree, TreeGetRoot(tree), left, 14) == -1)
        printf("no isert for you!\n");
    
    if (TreeInsertNode(tree, TreeGetRoot(tree), left, 15) == -1)
        printf("no isert for you!\n");

    if (TreeInsertNode(tree, TreeGetRoot(tree), right, 16) == -1)
        printf("no isert for you!\n");

    printf("<%d>\n", TreeFind(tree, TreeGetRoot(tree), 14));

    if (TreeDeleteNode(tree, TreeGetRoot(tree)) == -1)
        printf("no delete for you!\n"); 

    if (TreeDeleteNode(tree, TreeGetRoot(tree)) == -1)
        printf("no delete for you!\n");
    
    if (TreeInsertNode(tree, TreeGetRoot(tree), left, 13) == -1)
        printf("no insert for you!\n");

    if (TreeInsertNode(tree, TreeGetRoot(tree), left, 14) == -1)
        printf("no isert for you!\n");
    
    if (TreeInsertNode(tree, TreeGetRoot(tree), left, 15) == -1)
        printf("no isert for you!\n");

    if (TreeInsertNode(tree, TreeGetRoot(tree), right, 16) == -1)
        printf("no isert for you!\n");   

    if (TreeSort(tree) == -1)
        printf("no sort for you!\n");

    return 0;
}

//####################//


//####################//