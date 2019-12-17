#include <stdio.h>      // for printf
#include <stdlib.h>     // for calloc, free, atof
#include <unistd.h>     // for lseek
#include <sys/types.h>  // for open
#include <sys/stat.h>   // for open
#include <fcntl.h>      // for open
#include <ctype.h>      // for isdigit
#include <string.h>     // for strncmp

#include "diff.h"
#include "tree.h"

//####################//

const int PRECISION = 1000;

//####################//

struct _TreeMath* DiffReadExpression(const char* pathname) {
    struct _TreeMath* tree = TreeAlloc();

    TREE_INIT(tree, Math);

    int fd = 0;
    if ((fd = open(pathname, O_RDONLY)) == -1) {
        printf("[DiffReadExpression] ERROR: unable to open source file!\n");
        return NULL;
    }
    
    int len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char* buf = calloc(len, sizeof(char));

    if (read(fd, buf, len) != len) {
        printf("[DiffReadExpression] ERROR: unable to read from source file!\n");
        
        free(buf);
        TreeFree(tree);

        return NULL;
    }

    char* s = buf;

    // dirty hack, needs FIX /////////////
    MathData dataNew = {};              //
    INSERT(tree->free, Left, 9, 99);   //

    if (GetTier4(&s, tree, tree->root) == -1) {
        printf("[DiffReadExpression] ERROR: error while reading expression at pos %ld!\n", s - buf);
        
        free(buf);
        TreeFree(tree);

        return NULL;
    }

    free(buf);

    return tree;   
}

int GetTier4(char** s, struct _TreeMath* tree, int node) {
    GetSpace(s);

    if (GetGr(s, tree, node) == -1) {
        return -1;
    }

    return 0;   
}

int GetGr(char** s, struct _TreeMath* tree, int node) {
    if (GetTier3(s, tree, node, Left) == -1) {
        printf("    [GetGr] WARNING: expression identified as empty line!\n");
    }

    GetSpace(s);

    if (**s != '#') {
        printf("    [GetGr] ERROR: unexpected end symbol <%c>! expected <#> at the end!\n", **s);
        return -1;
    }

    return 0;
}

int GetTier3(char** s, struct _TreeMath* tree, int node, int branch) {
    GetSpace(s);

    if (GetSumSub(s, tree, node, branch) == -1) {
        return -1;
    }

    return 0;
}

int GetSumSub(char** s, struct _TreeMath* tree, int node, int branch) {
    if (GetTier2(s, tree, node, branch) == -1) {
        printf("    [GetSumSub] ERROR: GetTier2 executed with eror!\n");
        return -1;
    }

    int node_ = tree->nodes[node].branch[branch];
    //printf("    [GetSumSub] node_: %d, tree->cur: %d, node: %d\n", node_, tree->cur, node);

    GetSpace(s);

    while (**s == '+' || **s == '-') {
        int sign = (**s == '-') ? (Sub) : (Sum);

        *s += sizeof(char);

        Node* cpy = TreeCopySubtree(tree, node_);
        int nodesCount = TreeCountSubtree(tree, node_);
        TreeDeleteNode(tree, tree->nodes[node_].branch[Left]);
        TreeDeleteNode(tree, tree->nodes[node_].branch[Right]);

        struct _MathData data = {Operator, sign};

        TreeChangeNode(tree, node_, NULL, NULL, NULL, &data);

        if (TreeGlueSubtree(tree, cpy, node_, Left, nodesCount) == -1) {
            printf("    [GetSumSub] ERROR: unable to glue subtree!\n");
            return -1;
        }

        GetSpace(s);

        if (GetTier2(s, tree, node_, Right) == -1) {
            printf("    [GetSumSub] ERROR: GetTier2 executed with errors after successfully reading sign!\n");
            return -1;
        }

        GetSpace(s);

        switch (sign) {
            case Sum: node_ = tree->nodes[node_].branch[Right]; break;
            case Sub: node_ = tree->nodes[node_].branch[Left]; break;
        }
    }

    return 0;
}

int GetTier2(char** s, struct _TreeMath* tree, int node, int branch) {
    GetSpace(s);

    if (GetMulDiv(s, tree, node, branch) == -1) {
        return -1;
    }

    return 0;
}

int GetMulDiv(char** s, struct _TreeMath* tree, int node, int branch) {
    if (GetTier1(s, tree, node, branch) == -1) {
        printf("    [GetMulDiv] ERROR: GetTier1 executed with eror!\n");
        return -1;
    }

    int node_ = tree->nodes[node].branch[branch];
    //printf("    [GetMulDiv] node_: %d, tree->cur: %d, node: %d\n", node_, tree->cur, node);
    
    GetSpace(s);
    
    while (**s == '*' || **s == '/') {
        int sign = (**s == '*') ? (Mul) : (Div);

        *s += sizeof(char);

        Node* cpy = TreeCopySubtree(tree, node_);
        int nodesCount = TreeCountSubtree(tree, node_);
        TreeDeleteNode(tree, tree->nodes[node_].branch[Right]);
        TreeDeleteNode(tree, tree->nodes[node_].branch[Left]); 

        struct _MathData data = {Operator, sign};

        TreeChangeNode(tree, node_, NULL, NULL, NULL, &data);       

        if (TreeGlueSubtree(tree, cpy, node_, Left, nodesCount) == -1) {
            printf("    [GetMulDiv] ERROR: unable to glue subtree!\n");
            return -1;
        }

        GetSpace(s);

        if (GetTier1(s, tree, node_, Right) == -1) {
            printf("    [GetMulDiv] ERROR: GetTier1 executed with errors after successfully reading sign!\n");
            return -1;
        }

        GetSpace(s);

        switch (sign) {
            case Mul: node_ = tree->nodes[node_].branch[Right]; break;
            case Div: node_ = tree->nodes[node_].branch[Left]; break;
        }
    }

    return 0;
}

int GetTier1(char** s, struct _TreeMath* tree, int node, int branch) {
    GetSpace(s);
    
    if (GetPow(s, tree, node, branch) == -1) {
        return -1;
    }

    return 0;
}

int GetPow(char** s, struct _TreeMath* tree, int node, int branch) {
    if (GetTier0(s, tree, node, branch) == -1) {
        printf("    [GetPow] ERROR: GetTier0 executed with eror!\n");
        return -1;
    }

    int node_ = tree->nodes[node].branch[branch];
    //printf("    [GetPow] node_: %d, tree->cur: %d, node: %d\n", node_, tree->cur, node);

    GetSpace(s);
    
    while (**s == '^') {
        *s += sizeof(char);

        Node* cpy = TreeCopySubtree(tree, node_);
        int nodesCount = TreeCountSubtree(tree, node_);
        TreeDeleteNode(tree, tree->nodes[node_].branch[Left]);
        TreeDeleteNode(tree, tree->nodes[node_].branch[Right]);  

        struct _MathData data = {Operator, Pow};

        TreeChangeNode(tree, node_, NULL, NULL, NULL, &data);      

        if (TreeGlueSubtree(tree, cpy, node_, Left, nodesCount) == -1) {
            printf("    [GetPow] ERROR: unable to glue subtree!\n");
            return -1;
        }

        GetSpace(s);

        if (GetTier0(s, tree, node_, Right) == -1) {
            printf("    [GetPow] ERROR: GetTier0 executed with errors after successfully reading sign!\n");
            return -1;
        }

        GetSpace(s);

        node_ = tree->nodes[node_].branch[Right];
    }

    return 0;
}

int GetTier0(char** s, struct _TreeMath* tree, int node, int branch) {
    GetSpace(s);

    if (GetVar(s, tree, node, branch) != -1) {
        //printf("  [GetTier0] variable was read!\n");
        return 0;
    }
    
    if (GetNum(s, tree, node, branch) != -1) {
        //printf("  [GetTier0] number was read!\n");
        return 0;
    }

    if (GetPar(s, tree, node, branch) != -1) {
        //printf("  [GetTier0] parenthesis was read!\n");
        return 0;
    }
    
    if (GetFunc(s, tree, node, branch) != -1) {
        //printf("  [GetTier0] function was read!\n");
        return 0;
    }

    printf("  [GetTier0] ERROR: no valid tier0 expression was read!\n");
    return -1;
}

int GetVar(char** s, struct _TreeMath* tree, int node, int branch) {
    if (**s == 'x') {
        *s += sizeof(char);
        MathData dataNew = {};

        INSERT(node, branch, Variable, 'x');  

        return 0;
    } 
    // TODO: add multiple variables

    return -1;
}

int GetNum(char** s, struct _TreeMath* tree, int node, int branch) {
    int val = 0;

    if (strncmp(*s, "pi", 2) == 0) {
        val = (int)(3.14 * PRECISION);
        *s += 2 * sizeof(char);
    } else if (strncmp(*s, "e", 1) == 0) {
        val = (int)(2.73 * PRECISION);
        *s += sizeof(char);
    } else if (isdigit(**s) == 0 && **s != '-' && **s != '+') {
        return -1;
    } else {
        val = (int)(atof(*s) * PRECISION);
    }

    if (**s == '-' || **s == '+')
        **s += sizeof(char);
    
    while (isdigit(**s) != 0)
        *s += 1;                                             
                                                             
    if (**s == '.') {                                        
        *s += 1;                                             
        while (isdigit(**s) != 0)                            
            *s += 1;                                         
    }                                                       

    MathData dataNew = {};
    INSERT(node, branch, Number, val);

    return 0;
}

int GetPar(char** s, struct _TreeMath* tree, int node, int branch) {
    if (**s == '(') {
        *s += sizeof(char);
        GetSpace(s);

        if (GetTier3(s, tree, node, branch) == -1) {
            printf("    [GetPar] ERROR: invalid expression in parenthsis!\n");
        }

        GetSpace(s);

        if (**s == ')') {
            *s += sizeof(char);

            return 0;
        } else {
            printf("    [GetPar] ERROR: no closing parenthesis detected!\n");
            return -1;
        }
    }

    return -1;
}

int GetFunc(char** s, struct _TreeMath* tree, int node, int branch) {
    MathData dataNew = {};

    if (strncmp(*s, "sin", 3) == 0) {
        dataNew.value = Sin;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "cos", 3) == 0) {
        dataNew.value = Cos;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "tan", 3) == 0) {
        dataNew.value = Tan;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "ctan", 4) == 0) {
        dataNew.value = Ctan;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "asin", 4) == 0) {
        dataNew.value = Asin;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "acos", 4) == 0) {
        dataNew.value = Acos;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "atan", 4) == 0) {
        dataNew.value = Atan;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "actan", 5) == 0) {
        dataNew.value = Actan;

        *s += 5 * sizeof(char);
    } else if (strncmp(*s, "ln", 2) == 0) {
        dataNew.value = Ln;

        *s += 2 * sizeof(char);
    } else if (strncmp(*s, "log", 3) == 0) {
        dataNew.value = Log;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "sqrt", 4) == 0) {
        dataNew.value = Sqrt;

        *s += 4 * sizeof(char);
    } else {
        printf("    [GetFunc] ERROR: no viable function was read!\n");
        return -1;
    }

    INSERT(node, branch, Function, dataNew.value);
    
    int node_ = tree->nodes[node].branch[branch];
    
    if (GetTier0(s, tree, node_, Left) == -1) {
        printf("    [GetFunc] ERROR: unable to interpret given argument!\n");
        return -1;
    }

    return 0;
}

//=====================

int DiffGetDerivative(struct _TreeMath* tree) {
    if (DiffSimplify(tree) == -1) {
        printf("[DiffGetDerivative] ERROR: unable to simplify!\n");
        return -1;
    }
    
    if (DiffNode(tree, tree->nodes[tree->root].branch[Left]) == -1) {
        printf("[DiffGetDerivative] ERROR: unable to differentiate given tree!\n");
        return -1;
    }
    
    if (DiffSimplify(tree) == -1) {
        printf("[DiffGetDerivative] ERROR: unable to simplify!\n");
        return -1;
    }
    
    return 0;
}

int DiffNode(struct _TreeMath* tree, const int node) {
    switch (TYPE(node)) {
        case Variable: if (DiffVariable(tree, node) == -1) {return -1;} break;
        case Number:   if (DiffNumber(tree, node) == -1) {return -1;} break;
        case Function: if (DiffFunction(tree, node) == -1) {return -1;} break;
        case Operator: if (DiffOperator(tree, node) == -1) {return -1;} break;
        default: printf("[DiffNode] ERROR: invalid node type given!\n"); return -1; break;
    }

    return 0;
}

int DiffVariable(struct _TreeMath* tree, const int node) {
    printf("*090909\n");
    MathData dataNew = {Number, 1 * PRECISION};

    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);

    return 0;
}

int DiffNumber(struct _TreeMath* tree, const int node) {
    MathData dataNew = {Number, 0};

    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);

    return 0;
}

int DiffFunction(struct _TreeMath* tree, const int node) {
    // copying function's and argument's subtrees, clearing node's subtree;
    Node* argCpy = TreeCopySubtree(tree, L);
    int argCount = TreeCountSubtree(tree, L);
    Node* funcCpy = TreeCopySubtree(tree, node);
    int funcCount = TreeCountSubtree(tree, node);

    TreeDeleteNode(tree, L);
    TreeDeleteNode(tree, R);

    // replacing function's node with multiplication of func and it's argument, freeing temporary buffers
    MathData dataNew = {Operator, Mul};                     
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew); 

    TreeGlueSubtree(tree, funcCpy, node, Left, funcCount); 
    TreeGlueSubtree(tree, argCpy, node, Right, argCount);

    free(argCpy);   
    free(funcCpy);

    // getting func's derivative. these functions only change function's node, f.ex: sin(nx) -> cos(nx); cos(nx) -> -1 * sin(nx)
    switch (VALUE(L)) {
        case Sin:   if (DiffSin(tree, L) == -1) {return -1;} break;
        case Cos:   if (DiffCos(tree, L) == -1) {return -1;} break;
        case Tan:   if (DiffTan(tree, L) == -1) {return -1;} break;
        case Ctan:  if (DiffCtan(tree, L) == -1) {return -1;} break;
        case Asin:  if (DiffAsin(tree, L) == -1) {return -1;} break;
        case Acos:  if (DiffAcos(tree, L) == -1) {return -1;} break;
        case Atan:  if (DiffAtan(tree, L) == -1) {return -1;} break;
        case Actan: if (DiffActan(tree, L) == -1) {return -1;} break;
        case Log:   if (DiffLog(tree, L) == -1) {return -1;} break;
        case Ln:    if (DiffLn(tree, L) == -1) {return -1;} break;
        case Sqrt:  if (DiffSqrt(tree, L) == -1) {return -1;} break;
        default: printf("[DiffFunction] invalid function code!\n"); return -1; break;
    }
    
    // getting argument's derivative
    if (DiffNode(tree, R) == -1) {
        printf("[DiffFunction] ERROR: couldn't get argument's derivatie!\n");
        return -1;
    }

    return 0;
}

int DiffSin(struct _TreeMath* tree, const int node) {
    MathData dataNew = {Function, Cos};
    if (TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew) == -1)
        return -1;

    return 0;
}

int DiffCos(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};

    Node* argCpy = TreeCopySubtree(tree, L);
    int argCount = TreeCountSubtree(tree, L);
    
    TreeDeleteNode(tree, L);
    TreeDeleteNode(tree, R);

    CHANGE(node, Operator, Mul);
    INSERT(node, Right, Number, -1 * PRECISION);
    INSERT(node, Left, Function, Sin);

    TreeGlueSubtree(tree, argCpy, L, Left, argCount);

    free(argCpy);

    return 0;
}

int DiffTan(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};

    Node* argCpy = TreeCopySubtree(tree, L);
    int argCount = TreeCountSubtree(tree, L);

    CHANGE(node, Operator, Div);
    DELETE(R);
    DELETE(L);

    INSERT(node, Left, Number, 1 * PRECISION);
    INSERT(node, Right, Operator, Pow);
    INSERT(R, Right, Number, 2 * PRECISION);
    INSERT(R, Left, Function, Cos);

    TreeGlueSubtree(tree, argCpy, tree->nodes[R].branch[Left], Left, argCount);

    free(argCpy);

    return 0;
}

int DiffCtan(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};
    
    Node* argCpy = TreeCopySubtree(tree, L);
    int argCount = TreeCountSubtree(tree, L);

    CHANGE(node, Operator, Div);
    DELETE(R);
    DELETE(L);

    INSERT(node, Left, Number, -1 * PRECISION);
    INSERT(node, Right, Operator, Pow);
    INSERT(R, Right, Number, 2 * PRECISION);
    INSERT(R, Left, Function, Sin);

    TreeGlueSubtree(tree, argCpy, tree->nodes[R].branch[Left], Left, argCount);

    free(argCpy);

    return 0;
}
// TODO
int DiffAsin(struct _TreeMath* tree, const int node) {
    printf("[DiffAsin] still in active development! come back later!\n");

    return -1;
}
// TODO
int DiffAcos(struct _TreeMath* tree, const int node) {
    printf("[DiffAcos] still in active development! come back later!\n");

    return -1;
}
// TODO
int DiffAtan(struct _TreeMath* tree, const int node) {
    printf("[DiffAtan] still in active development! come back later!\n");

    return -1;
}
// TODO
int DiffActan(struct _TreeMath* tree, const int node) {
    printf("[DiffActan] still in active development! come back later!\n");

    return -1;
}

int DiffLog(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};

    Node* argCpy = TreeCopySubtree(tree, tree->nodes[node].branch[Left]);
    int argCount = TreeCountSubtree(tree, tree->nodes[node].branch[Left]);
    
    CHANGE(node, Operator, Div);
    DELETE(R);
    DELETE(L);

    INSERT(node, Left, Number, 1 * PRECISION);
    INSERT(node, Right, Operator, Mul);

    TreeGlueSubtree(tree, argCpy, R, Left, argCount);

    INSERT(R, Right, Function, Ln);
    INSERT(RR, Left, Number, 10 * PRECISION);

    free(argCpy);

    return 0;
}

int DiffLn(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};

    Node* argCpy = TreeCopySubtree(tree, tree->nodes[node].branch[Left]);
    int argCount = TreeCountSubtree(tree, tree->nodes[node].branch[Left]);
    
    CHANGE(node, Operator, Div);
    DELETE(R);
    DELETE(L);

    INSERT(node, Left, Number, 1 * PRECISION);

    TreeGlueSubtree(tree, argCpy, node, Right, argCount);

    free(argCpy);

    return 0;
}

int DiffSqrt(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};

    Node* funcCpy = TreeCopySubtree(tree, node);
    int funcCount = TreeCountSubtree(tree, node);

    CHANGE(node, Operator, Div);
    DELETE(R);
    DELETE(L);

    INSERT(node, Left, Number, 1 * PRECISION);
    INSERT(node, Right, Operator, Mul);
    INSERT(R, Left, Number, 2 * PRECISION);

    TreeGlueSubtree(tree, funcCpy, R, Right, funcCount);

    free(funcCpy);

    return 0;
}

int DiffOperator(struct _TreeMath* tree, const int node) {
    // getting func's derivative. these functions only change function's node, f.ex: sin(nx) -> cos(nx); cos(nx) -> -1 * sin(nx)
    switch (VALUE(node)) {
        case Mul: if (DiffMul(tree, node) == -1) {return -1;} break;
        case Div: if (DiffDiv(tree, node) == -1) {return -1;} break;
        case Sum: if (DiffSumSub(tree, node) == -1) {return -1;} break;
        case Sub: if (DiffSumSub(tree, node) == -1) {return -1;} break;
        case Pow: if (DiffPow(tree, node) == -1) {return -1;} break;
        default: printf("[DiffOperator] invalid operator code!\n"); return -1; break;
    }

    return 0;
}

int DiffMul(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};
    
    Node* leftCpy = TreeCopySubtree(tree, L);
    int leftCount = TreeCountSubtree(tree, L);
    Node* rightCpy = TreeCopySubtree(tree, R);
    int rightCount = TreeCountSubtree(tree, R);

    CHANGE(node, Operator, Sum);
    DELETE(L);
    DELETE(R);

    INSERT(node, Left, Operator, Mul);
    INSERT(node, Right, Operator, Mul);

    TreeGlueSubtree(tree, leftCpy, L, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, L, Right, rightCount);
    TreeGlueSubtree(tree, leftCpy, R, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, R, Right, rightCount);

    if (D(LL) == -1 || D(RR) == -1)
        return -1;

    free(leftCpy);
    free(rightCpy);

    return 0;
}

int DiffDiv(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};

    Node* leftCpy = TreeCopySubtree(tree, L);
    int leftCount = TreeCountSubtree(tree, L);
    Node* rightCpy = TreeCopySubtree(tree, R);
    int rightCount = TreeCountSubtree(tree, R);

    DELETE(L);
    DELETE(R);

    INSERT(node, Right, Operator, Mul);
    INSERT(node, Left, Operator, Sub);
    
    TreeGlueSubtree(tree, rightCpy, R, Right, rightCount);
    TreeGlueSubtree(tree, rightCpy, R, Left, rightCount);

    INSERT(L, Left, Operator, Mul);
    INSERT(L, Right, Operator, Mul);

    TreeGlueSubtree(tree, leftCpy, LL, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, LL, Right, rightCount);

    TreeGlueSubtree(tree, leftCpy, LR, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, LR, Right, rightCount);

    if (D(tree->nodes[LL].branch[Left]) == -1 || D(tree->nodes[LR].branch[Right]) == -1)
        return -1;

    free(leftCpy);
    free(rightCpy);

    return 0;
}

int DiffSumSub(struct _TreeMath* tree, const int node) {
    if (D(L) == -1 || D(R) == -1)
        return -1;

    return 0;
}

int DiffPow(struct _TreeMath* tree, const int node) {
    MathData dataNew = {};

    Node* powCpy = TreeCopySubtree(tree, node);
    int powCount = TreeCountSubtree(tree, node);
    Node* leftCpy = TreeCopySubtree(tree, L);
    int leftCount = TreeCountSubtree(tree, L);
    Node* rightCpy = TreeCopySubtree(tree, R);
    int rightCount = TreeCountSubtree(tree, R);

    CHANGE(node, Operator, Mul);
    DELETE(L);
    DELETE(R);
    
    INSERT(node, Right, Operator, Mul);

    TreeGlueSubtree(tree, powCpy, node, Left, powCount);
    TreeGlueSubtree(tree, rightCpy, R, Right, rightCount);

    INSERT(R, Left, Function, Ln);

    TreeGlueSubtree(tree, leftCpy, RL, Left, leftCount);

    if (D(R) == -1)
        return -1;

    free(leftCpy);
    free(rightCpy);
    free(powCpy);

    return 0;
}

//=====================

int DiffSimplify(struct _TreeMath* tree) {
    int flagModified = 1;

    while (flagModified == 1) {
        flagModified = 0;

        if (_DiffSimplify(tree, tree->nodes[tree->root].branch[Left], &flagModified) == -1) {
            printf("[DiffSimplify] error while simplifying!\n");
            return -1;
        }
    }

    return 0;
}

int _DiffSimplify(struct _TreeMath* tree, const int node, int* flag) {
    if (node != 0) {
        if (_DiffSimplify(tree, L, flag) == -1 || _DiffSimplify(tree, R, flag) == -1)
            return -1;

        switch (TYPE(node)) {
            case Operator: if (SimplifyOperator(tree, node, flag) == -1) {return -1;} break;
            case Function: if (SimplifyFunction(tree, node, flag) == -1) {return -1;} break;
        }
    }

    return 0;
}

int SimplifyOperator(struct _TreeMath* tree, const int node, int* flag) {
    switch (VALUE(node)) {
        case Mul: if (SimplifyMul(tree, node, flag) == -1) {return -1;} break;
        case Div: if (SimplifyDiv(tree, node, flag) == -1) {return -1;} break;
        case Sum: if (SimplifySum(tree, node, flag) == -1) {return -1;} break;
        case Sub: if (SimplifySub(tree, node, flag) == -1) {return -1;} break;
    }

    return 0;
}

int SimplifyMul(struct _TreeMath* tree, const int node, int* flag) {
    int parent = tree->nodes[node].parent;
    int parentBranch = -1;
    if (tree->nodes[node].parent != 0) {
        if (tree->nodes[tree->nodes[node].parent].branch[Left] == node)
            parentBranch = Left;
        else
            parentBranch = Right;
    }
    
    if ((TYPE(L) == Number && VALUE(L) == 0) || (TYPE(R) == Number && VALUE(R) == 0)) {
        MathData dataNew = {};
        CHANGE(node, Number, 0);

        DELETE(L);
        DELETE(R);

        *flag = 1;
    } else if (TYPE(L) == Number && VALUE(L) == 1 * PRECISION) {
        Node* rightCpy = TreeCopySubtree(tree, R);
        int rightCount = TreeCountSubtree(tree, R);

        DELETE(node);

        TreeGlueSubtree(tree, rightCpy, parent, parentBranch, rightCount);

        free(rightCpy);

        *flag = 1;
    } else if (TYPE(R) == Number && VALUE(R) == 1 * PRECISION) {
        Node* leftCpy = TreeCopySubtree(tree, L);
        int leftCount = TreeCountSubtree(tree, L);

        DELETE(node);

        TreeGlueSubtree(tree, leftCpy, parent, parentBranch, leftCount);

        free(leftCpy);

        *flag = 1;
    } else if (TYPE(R) == Number && TYPE(L) == Number) {
        MathData dataNew = {};
        CHANGE(node, Number, (VALUE(L) * VALUE(R)) * PRECISION);

        DELETE(L);
        DELETE(R);

        *flag = 1;
    }

    return 0;
}

int SimplifyDiv(struct _TreeMath* tree, const int node, int* flag) {
    if (TYPE(L) == Number && VALUE(L) == 0) {
        MathData dataNew = {};
        CHANGE(node, Number, 0);

        DELETE(L);
        DELETE(R);

        *flag = 1;
    } else if (TYPE(R) == Number && VALUE(R) == 1 * PRECISION) {
        int parent = tree->nodes[node].parent;
        int parentBranch = -1;

        if (tree->nodes[node].parent != 0) {
            if (tree->nodes[tree->nodes[node].parent].branch[Left] == node)
                parentBranch = Left;
            else
                parentBranch = Right;
        }

        Node* leftCpy = TreeCopySubtree(tree, L);
        int leftCount = TreeCountSubtree(tree, L);

        DELETE(node);

        TreeGlueSubtree(tree, leftCpy, parent, parentBranch, leftCount);

        free(leftCpy);

        *flag = 1;
    } else if (TYPE(R) == Number && TYPE(L) == Number) {
        MathData dataNew = {};
        CHANGE(node, Number, (VALUE(L) / VALUE(R)) * PRECISION);

        DELETE(L);
        DELETE(R);

        *flag = 1;
    }

    return 0;
}

int SimplifySum(struct _TreeMath* tree, const int node, int* flag) {
    int parent = tree->nodes[node].parent;
    int parentBranch = -1;
    if (tree->nodes[node].parent != 0) {
        if (tree->nodes[tree->nodes[node].parent].branch[Left] == node)
            parentBranch = Left;
        else
            parentBranch = Right;
    }
    
    if (TYPE(L) == Number && VALUE(L) == 0) {
        Node* rightCpy = TreeCopySubtree(tree, R);
        int rightCount = TreeCountSubtree(tree, R);

        DELETE(node);

        TreeGlueSubtree(tree, rightCpy, parent, parentBranch, rightCount);

        free(rightCpy);

        *flag = 1;
    } else if (TYPE(R) == Number && VALUE(R) == 0) {
        Node* leftCpy = TreeCopySubtree(tree, L);
        int leftCount = TreeCountSubtree(tree, L);

        DELETE(node);

        TreeGlueSubtree(tree, leftCpy, parent, parentBranch, leftCount);

        free(leftCpy);

        *flag = 1;
    } else if (TYPE(R) == Number && TYPE(L) == Number) {
        MathData dataNew = {};
        CHANGE(node, Number, (VALUE(L) + VALUE(R)) * PRECISION);

        DELETE(L);
        DELETE(R);

        *flag = 1;
    }

    return 0;
}

int SimplifySub(struct _TreeMath* tree, const int node, int* flag) {
    if (TYPE(L) == Number && VALUE(L) == 0) {
        int parent = tree->nodes[node].parent;
        int parentBranch = -1;
        if (tree->nodes[node].parent != 0) {
            if (tree->nodes[tree->nodes[node].parent].branch[Left] == node)
                parentBranch = Left;
            else
                parentBranch = Right;
        }

        Node* rightCpy = TreeCopySubtree(tree, R);
        int rightCount = TreeCountSubtree(tree, R);

        DELETE(node);

        TreeGlueSubtree(tree, rightCpy, parent, parentBranch, rightCount);

        free(rightCpy);

        *flag = 1;
    } else if (TYPE(R) == Number && VALUE(R) == 0) {
        MathData dataNew = {};
        CHANGE(node, Operator, Mul);
        DELETE(R);
        INSERT(node, Right, Number, -1 * PRECISION);

        *flag = 1;
    } else if (TYPE(R) == Number && TYPE(L) == Number) {
        MathData dataNew = {};
        CHANGE(node, Number, (VALUE(L) - VALUE(R)) * PRECISION);

        DELETE(L);
        DELETE(R);

        *flag = 1;
    }

    return 0;
}

int SimplifyFunction(struct _TreeMath* tree, const int node, int* flag) {
    switch (VALUE(node)) {
        case Sin:  if (SimplifySin(tree, node, flag) == -1) {return -1;} break;
        case Cos:  if (SimplifyCos(tree, node, flag) == -1) {return -1;} break;
        case Tan:  if (SimplifyTan(tree, node, flag) == -1) {return -1;} break;
    }
    
   return 0;
}

int SimplifySin(struct _TreeMath* tree, const int node, int* flag) {
    if ((TYPE(L) == Number && VALUE(L) == 3.14 * PRECISION) || (TYPE(L) == Number && VALUE(L) == 0)) {
        MathData dataNew = {};

        DELETE(L);
        CHANGE(node, Number, 0);

        *flag = 1;
    }

    return 0;
}

int SimplifyCos(struct _TreeMath* tree, const int node, int* flag) {
    if ((TYPE(L) == Number && VALUE(L) == 0) || (TYPE(L) == Number && VALUE(L) == 3.14 * PRECISION)) {
        MathData dataNew = {};

        DELETE(L);
        CHANGE(node, Number, 1 * PRECISION);

        *flag = 1;
    }

    return 0;
}

int SimplifyTan(struct _TreeMath* tree, const int node, int* flag) {
    if ((TYPE(L) == Number && VALUE(L) == 3.14 * PRECISION) || (TYPE(L) == Number && VALUE(L) == 0)) {
        MathData dataNew = {};

        DELETE(L);
        CHANGE(node, Number, 0);

        *flag = 1;
    }

    return 0;
}

//=====================

int DiffPrintTree(struct _TreeMath* tree) {
    int fd = open("output/report.tex", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    
    if (fd == -1) {
        printf("[DiffPintTree] ERROR: unable to open report file!\n");
        return -1;
    }

    TREE_VERIFY(tree);

    dprintf(fd, "\\documentclass[a4paper]{article}\n"
                "\\usepackage[T2A]{fontenc}\n"
                "\\usepackage[utf8]{inputenc}\n"
                "\\usepackage[english,russian]{babel}\n"
                "\\usepackage{wrapfig}\n"
                "\\usepackage{amsmath,amsfonts,amssymb,amsthm,mathtools}\n"
                "\\usepackage[pdftex]{graphicx}\n"
                "\\graphicspath{{pictures/}}\n"
                "\\DeclareGraphicsExtensions{.pdf,.png,.jpg}\n"
                "\\usepackage[left=3cm,right=3cm,\n"
                "  top=3.5cm,bottom=2cm,bindingoffset=0cm]{geometry}\n"
                "\\usepackage{wrapfig}\n"
                "\\usepackage{float}\n"
                "\\usepackage{graphicx}\n"
                "\\begin{document}\n"
                "\\newpage\n"
                "\n\\[");

    if (_DiffPrintTree(fd, tree, tree->nodes[tree->root].branch[Left]) == -1) {
        printf("[DiffPrintTree] ERROR: error while writing tree to the output!\n");
        return -1;
    }

    dprintf(fd, "\\] \n\\end{document}\n");

    system("pdflatex output/report.tex");

    return 0;
}

int _DiffPrintTree(const int fd, struct _TreeMath* tree, const int node) {
    if (node != 0) {
        switch (TYPE(node)) {
            case Variable: if (PrintVariable(fd, tree, node) == -1) {return -1;} break;
            case Number:   if (PrintNumber(fd, tree, node) == -1) {return -1;} break;
            case Operator: if (PrintOperator(fd, tree, node) == -1) {return -1;} break;
            case Function: if (PrintFunction(fd, tree, node) == -1) {return -1;} break;
            default: printf("[DiffPrintTree] invalid node type!\n"); return -1; break;
        }
    }

    return 0;
}

int PrintVariable(const int fd, struct _TreeMath* tree, const int node) {
    dprintf(fd, "x");

    return 0;
}

int PrintNumber(const int fd, struct _TreeMath* tree, const int node) {
    if (VALUE(node) == 3140)
        dprintf(fd, "\\pi");
    else if (VALUE(node) == 2730)
        dprintf(fd, "e");
    else if (VALUE(node) >= 0)
        dprintf(fd, "%0.2f", (double)(VALUE(node)) / PRECISION);
    else
        dprintf(fd, "(%0.2f)", (double)(VALUE(node)) / PRECISION); 

    return 0;
}

int PrintOperator(const int fd, struct _TreeMath* tree, const int node) {
    switch (VALUE(node)) {
        case Sum: if (PrintSum(fd, tree, node) == -1) {return -1;} break;
        case Sub: if (PrintSub(fd, tree, node) == -1) {return -1;} break;
        case Mul: if (PrintMul(fd, tree, node) == -1) {return -1;} break;
        case Div: if (PrintDiv(fd, tree, node) == -1) {return -1;} break;
        case Pow: if (PrintPow(fd, tree, node) == -1) {return -1;} break;
    }

    return 0;
}

int PrintSum(const int fd, struct _TreeMath* tree, const int node) {
    if (_DiffPrintTree(fd, tree, L) == -1)
        return -1;

    dprintf(fd, " + ");

    if (_DiffPrintTree(fd, tree, R) == -1)
        return -1;

    return 0;
}

int PrintSub(const int fd, struct _TreeMath* tree, const int node) {
    if (_DiffPrintTree(fd, tree, L) == -1)
        return -1;

    dprintf(fd, " - ");

    if (TYPE(R) == Operator && (VALUE(R) == Sub || VALUE(R) == Sum)) {
        dprintf(fd, " \\left( ");

        if (_DiffPrintTree(fd, tree, R) == -1)
            return -1;

        dprintf(fd, " \\right) ");
    } else {
        if (_DiffPrintTree(fd, tree, R) == -1)
            return -1;
    }

    return 0;
}

int PrintMul(const int fd, struct _TreeMath* tree, const int node) {
    if (TYPE(L) == Operator && (VALUE(L) == Sub || VALUE(L) == Sum || VALUE(L) == Div)) {
        dprintf(fd, " \\left( ");

        if (_DiffPrintTree(fd, tree, L) == -1)
            return -1;

        dprintf(fd, " \\right) ");
    } else {
        if (_DiffPrintTree(fd, tree, L) == -1)
            return -1;
    }

    dprintf(fd, " \\cdot ");
    
    if (VALUE(R) == Operator && (VALUE(R) == Sub || VALUE(R) == Sum)) {
        dprintf(fd, " \\left( ");

        if (_DiffPrintTree(fd, tree, R) == -1)
            return -1;

        dprintf(fd, " \\right) ");
    } else {
        if (_DiffPrintTree(fd, tree, R) == -1)
            return -1;
    }

    return 0;
}

int PrintDiv(const int fd, struct _TreeMath* tree, const int node) {
    dprintf(fd, "\\dfrac{ ");

    if (_DiffPrintTree(fd, tree, L) == -1)
        return -1;

    dprintf(fd, " }{ ");

    if (_DiffPrintTree(fd, tree, R) == -1)
        return -1;

    dprintf(fd, " } ");

    return 0;
}

int PrintPow(const int fd, struct _TreeMath* tree, const int node) {
    dprintf(fd, " \\left( ");    

    if (_DiffPrintTree(fd, tree, L) == -1)
        return -1;

    dprintf(fd, " \\right)^{");

    if (_DiffPrintTree(fd, tree, R) == -1)
        return -1;

    dprintf(fd, " } ");

    return 0;
}

int PrintFunction(const int fd, struct _TreeMath* tree, const int node) {
    switch (VALUE(node)) {
        case Sin:   dprintf(fd, "\\sin{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Cos:   dprintf(fd, "\\cos{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Tan:   dprintf(fd, "\\tan{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Ctan:  dprintf(fd, "\\ctan{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Asin:  dprintf(fd, "\\arcsin{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Acos:  dprintf(fd, "\\arccos{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Atan:  dprintf(fd, "\\arctan{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Actan: dprintf(fd, "\\arcctan{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Log:   dprintf(fd, "\\log{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Ln:    dprintf(fd, "\\ln{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Sqrt:  dprintf(fd, "\\sqrt{ \\left("); if (_DiffPrintTree(fd, tree, L) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
    }

    return 0;
}
