#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "diff.h"

//####################//

int main() {
    //Tree* tree = TreeRead("input/database.txt", Math);

    Tree* tree = DiffReadExpression("input/expression.txt");

    //printf("res: %d\n", DiffPrintTree(tree));

    if (DiffGetDerivative(tree) == -1) {
        printf("-1\n");
        return -1;
    }

    DiffPrintTree(tree);

    TreeFree(tree);

    return 0;
}
