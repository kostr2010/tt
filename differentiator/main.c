#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "diff.h"

//####################//



//####################//

int main() {
    //Tree* tree = TreeRead("input/database.txt", Math);

    Tree* tree = DiffReadExpression("input/expression.txt");

    printf("res: %d\n", DiffPrintTree(tree));

    DiffGetDerivative(tree);

    DiffPrintTree(tree);

    //system("pdflatex output/report.tex");

    //TreeSort(tree);

    TreeFree(tree);

    return 0;
}

//####################//



//####################//