// Tiers are from in descending order (0 - first priority, 5 - last priority)
// 
// TIER0:   NUM | VAR | PAR | FUNC
//          NUM:    [0-9]+ | [pi, e, i]
//          VAR:    [x]
//          PAR:    '(' TIER3 ')'
//          FUNC:   [sin, cos, ln...] '(' TIER3 ')'
// 
// TIER1:   POW
//          POW:    TIER0 {'^' TIER0}*
// 
// TIER2:   MULDIV
//          MULDIV: TIER1 {[*, /] TIER1}*
// 
// TIER3:   SUMSUB
//          SUMSUB: TIER2 {[+, -] TIER2}*
// 
// TIER4:   GR
//          GR:     TIER3 #
//

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

    MathData dataStart = {999, 999};
    TreeInsertNode(tree, tree->free, Left, &dataStart);
    int node = tree->root;
    //printf("node: %d\n", node);

    if (GetTier4(&s, tree, node) == -1) {
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
    //int node_ = (tree->cur > 1) ? (tree->nodes[node].branch[branch]) : (tree->root);

    if (GetTier2(s, tree, node, branch) == -1) {
        printf("    [GetSumSub] ERROR: GetTier2 executed with eror!\n");
        return -1;
    }

    int node_ = tree->nodes[node].branch[branch];
    //int node_ = (node == tree->root/* && tree->cur < 2*/) ? (tree->root) : (tree->nodes[node].branch[branch]);
    //node_ = (node_ == tree->root) ? (tree->nodes[node].branch[branch]) : (tree->root);
    printf("    [GetSumSub] node_: %d, tree->cur: %d, node: %d\n", node_, tree->cur, node);

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

        //tree->nodes[node_].data.type = Operator;
        //tree->nodes[node_].data.type = sign;

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

        //node_ = tree->nodes[node_].branch[Left];
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
    //int node_ = (tree->cur > 1) ? (tree->nodes[node].branch[branch]) : (tree->root);

    if (GetTier1(s, tree, node, branch) == -1) {
        printf("    [GetMulDiv] ERROR: GetTier1 executed with eror!\n");
        return -1;
    }

    GetSpace(s);

    int node_ = tree->nodes[node].branch[branch];
    //int node_ = (node == tree->root/* && tree->cur < 2*/) ? (tree->root) : (tree->nodes[node].branch[branch]);
    //node_ = (node_ == tree->root) ? (tree->nodes[node].branch[branch]) : (tree->root);
    printf("    [GetMulDiv] node_: %d, tree->cur: %d, node: %d\n", node_, tree->cur, node);
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

        //tree->nodes[node_].data.type = Operator;
        //tree->nodes[node_].data.type = sign;

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

        //node_ = tree->nodes[node_].branch[Left];
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
    //int node_ = (tree->cur > 1) ? (tree->nodes[node].branch[branch]) : (tree->root);

    if (GetTier0(s, tree, node, branch) == -1) {
        printf("    [GetPow] ERROR: GetTier0 executed with eror!\n");
        return -1;
    }

    GetSpace(s);

    int node_ = tree->nodes[node].branch[branch];
    //int node_ = (node == tree->root/* && tree->cur < 2*/) ? (tree->root) : (tree->nodes[node].branch[branch]);
    //node_ = (node_ == tree->root) ? (tree->nodes[node].branch[branch]) : (tree->root);
    printf("    [GetPow] node_: %d, tree->cur: %d, node: %d\n", node_, tree->cur, node);

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

        //tree->nodes[node_].data.type = Operator;
        //tree->nodes[node_].data.type = Pow;

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
        printf("  [GetTier0] variable was read!\n");
        return 0;
    }
    
    if (GetNum(s, tree, node, branch) != -1) {
        printf("  [GetTier0] number was read!\n");
        return 0;
    }
    
    if (GetPar(s, tree, node, branch) != -1) {
        printf("  [GetTier0] parenthesis was read!\n");
        return 0;
    }
    
    if (GetFunc(s, tree, node, branch) != -1) {
        printf("  [GetTier0] function was read!\n");
        return 0;
    }

    printf("  [GetTier0] ERROR: no valid tier1 expression was read!\n");
    return -1;
}

int GetVar(char** s, struct _TreeMath* tree, int node, int branch) {
    if (**s == 'x') {
        *s += sizeof(char);
        MathData data = {Variable, 'x'};
        TreeInsertNode(tree, node, branch, &data);  

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
    
    while (isdigit(**s) != 0 || **s == '-' || **s == '+')   // skipping number
        *s += 1;                                            // 
                                                            // 
    if (**s == '.') {                                       // 
        *s += 1;                                            // 
        while (isdigit(**s) != 0)                           // 
            *s += 1;                                        // 
    }                                                       // 

    MathData data = {Number, val};
    TreeInsertNode(tree, node, branch, &data);

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
    MathData data = {Function, -1};

    if (strncmp(*s, "sin", 3) == 0) {
        data.value = Sin;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "cos", 3) == 0) {
        data.value = Cos;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "tan", 3) == 0) {
        data.value = Tan;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "ctan", 4) == 0) {
        data.value = Ctan;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "asin", 4) == 0) {
        data.value = Asin;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "acos", 4) == 0) {
        data.value = Acos;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "atan", 4) == 0) {
        data.value = Atan;

        *s += 4 * sizeof(char);
    } else if (strncmp(*s, "actan", 5) == 0) {
        data.value = Actan;

        *s += 5 * sizeof(char);
    } else if (strncmp(*s, "ln", 2) == 0) {
        data.value = Ln;

        *s += 2 * sizeof(char);
    } else if (strncmp(*s, "log", 3) == 0) {
        data.value = Log;

        *s += 3 * sizeof(char);
    } else if (strncmp(*s, "sqrt", 4) == 0) {
        data.value = Sqrt;

        *s += 4 * sizeof(char);
    } else {
        //printf("    [GetFunc] ERROR: no viable function was read!\n");
        return -1;
    }

    TreeInsertNode(tree, node, branch, &data);
    
    int node_ = tree->nodes[node].branch[branch];
    //int node_ = (node == tree->root/* && tree->cur < 2*/) ? (tree->root) : (tree->nodes[node].branch[branch]);
    //int node_ = (tree->cur > 1) ? (tree->nodes[node].branch[branch]) : (tree->root);
    //int node_ = (node == tree->root) ? (tree->nodes[node].branch[branch]) : (tree->root);
    printf("node_ %d, node %d, tree->root %d\n", node_, node, tree->root);
    
    if (GetTier0(s, tree, node_, Left) == -1) {
        printf("    [GetFunc] ERROR: unable to interpret given argument!\n");
        return -1;
    }

    return 0;
}

int DiffGetDerivative(struct _TreeMath* tree) {
    if (DiffNode(tree, tree->nodes[tree->root].branch[Left]) == -1) {
        printf("[DiffGetDerivative] ERROR: unable to differentiate given tree!\n");
        return -1;
    }

    return 0;
}

int DiffNode(struct _TreeMath* tree, const int node) {
    switch (tree->nodes[node].data.type) {
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
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    // copying function's and argument's subtrees, clearing node's subtree;
    Node* argCpy = TreeCopySubtree(tree, branchL);
    int argCount = TreeCountSubtree(tree, branchL);
    Node* funcCpy = TreeCopySubtree(tree, node);
    int funcCount = TreeCountSubtree(tree, node);

    TreeDeleteNode(tree, branchL);
    TreeDeleteNode(tree, branchR);

    // replacing function's node with multiplication of func and it's argument, freeing temporary buffers
    MathData dataNew = {Operator, Mul};                     
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew); 

    TreeGlueSubtree(tree, funcCpy, node, Left, funcCount); 
    TreeGlueSubtree(tree, argCpy, node, Right, argCount);

    branchL = tree->nodes[node].branch[Left];
    branchR = tree->nodes[node].branch[Right];

    free(argCpy);   
    free(funcCpy);

    // getting func's derivative. these functions only change function's node, f.ex: sin(nx) -> cos(nx); cos(nx) -> -1 * sin(nx)
    switch (tree->nodes[branchL].data.value) {
        case Sin:   if (DiffSin(tree, branchL) == -1) {return -1;} break;
        case Cos:   if (DiffCos(tree, branchL) == -1) {return -1;} break;
        case Tan:   if (DiffTan(tree, branchL) == -1) {return -1;} break;
        case Ctan:  if (DiffCtan(tree, branchL) == -1) {return -1;} break;
        case Asin:  if (DiffAsin(tree, branchL) == -1) {return -1;} break;
        case Acos:  if (DiffAcos(tree, branchL) == -1) {return -1;} break;
        case Atan:  if (DiffAtan(tree, branchL) == -1) {return -1;} break;
        case Actan: if (DiffActan(tree, branchL) == -1) {return -1;} break;
        case Log:   if (DiffLog(tree, branchL) == -1) {return -1;} break;
        case Ln:    if (DiffLn(tree, branchL) == -1) {return -1;} break;
        case Sqrt:  if (DiffSqrt(tree, branchL) == -1) {return -1;} break;
        default: printf("[DiffFunction] invalid function code!\n"); return -1; break;
    }
    
    // getting argument's derivative
    if (DiffNode(tree, branchR) == -1) {
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
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    Node* argCpy = TreeCopySubtree(tree, branchL);
    int argCount = TreeCountSubtree(tree, branchL);
    
    TreeDeleteNode(tree, branchL);
    TreeDeleteNode(tree, branchR);

    MathData dataNew = {Operator, Mul};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);

    dataNew.type = Number;
    dataNew.value = -1 * PRECISION;
    TreeInsertNode(tree, node, Right, &dataNew);

    dataNew.type = Function;
    dataNew.value = Sin;
    TreeInsertNode(tree, node, Left, &dataNew);

    branchL = tree->nodes[node].branch[Left];
    branchR = tree->nodes[node].branch[Right];

    TreeGlueSubtree(tree, argCpy, branchL, Left, argCount);

    free(argCpy);

    return 0;
}

int DiffTan(struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];

    Node* argCpy = TreeCopySubtree(tree, branchL);
    int argCount = TreeCountSubtree(tree, branchL);
    
    MathData dataNew = {Operator, Div};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);
    TreeDeleteNode(tree, tree->nodes[node].branch[Right]);
    TreeDeleteNode(tree, tree->nodes[node].branch[Left]);

    dataNew.type = Number;
    dataNew.value = 1 * PRECISION;
    TreeInsertNode(tree, node, Left, &dataNew);

    dataNew.type = Operator;
    dataNew.value = Pow;
    TreeInsertNode(tree, node, Right, &dataNew);

    int branchR = tree->nodes[node].branch[Right];

    dataNew.type = Number;
    dataNew.value = 2 * PRECISION;
    TreeInsertNode(tree, branchR, Right, &dataNew);

    dataNew.type = Function;
    dataNew.value = Cos;
    TreeInsertNode(tree, branchR, Left, &dataNew);

    TreeGlueSubtree(tree, argCpy, tree->nodes[branchR].branch[Left], Left, argCount);

    free(argCpy);

    return 0;
}

int DiffCtan(struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];

    Node* argCpy = TreeCopySubtree(tree, branchL);
    int argCount = TreeCountSubtree(tree, branchL);
    
    MathData dataNew = {Operator, Div};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);
    TreeDeleteNode(tree, tree->nodes[node].branch[Right]);
    TreeDeleteNode(tree, tree->nodes[node].branch[Left]);

    dataNew.type = Number;
    dataNew.value = -1 * PRECISION;
    TreeInsertNode(tree, node, Left, &dataNew);

    dataNew.type = Operator;
    dataNew.value = Pow;
    TreeInsertNode(tree, node, Right, &dataNew);

    int branchR = tree->nodes[node].branch[Right];

    dataNew.type = Number;
    dataNew.value = 2 * PRECISION;
    TreeInsertNode(tree, branchR, Right, &dataNew);

    dataNew.type = Function;
    dataNew.value = Sin;
    TreeInsertNode(tree, branchR, Left, &dataNew);

    TreeGlueSubtree(tree, argCpy, tree->nodes[branchR].branch[Left], Left, argCount);

    free(argCpy);

    return 0;

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
    Node* argCpy = TreeCopySubtree(tree, tree->nodes[node].branch[Left]);
    int argCount = TreeCountSubtree(tree, tree->nodes[node].branch[Left]);
    
    MathData dataNew = {Operator, Div};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);
    TreeDeleteNode(tree, tree->nodes[node].branch[Right]);
    TreeDeleteNode(tree, tree->nodes[node].branch[Left]);

    dataNew.type = Number;
    dataNew.value = 1 * PRECISION;
    TreeInsertNode(tree, node, Left, &dataNew);

    dataNew.type = Operator;
    dataNew.value = Mul;
    TreeInsertNode(tree, node, Right, &dataNew);

    int branchR = tree->nodes[node].branch[Right];

    TreeGlueSubtree(tree, argCpy, branchR, Left, argCount);

    dataNew.type = Function;
    dataNew.value = Ln;
    TreeInsertNode(tree, branchR, Right, &dataNew);

    dataNew.type = Number;
    dataNew.value = 10 * PRECISION;
    TreeInsertNode(tree, tree->nodes[branchR].branch[Right], Left, &dataNew);

    free(argCpy);

    return 0;

    return 0;
}

int DiffLn(struct _TreeMath* tree, const int node) {
    Node* argCpy = TreeCopySubtree(tree, tree->nodes[node].branch[Left]);
    int argCount = TreeCountSubtree(tree, tree->nodes[node].branch[Left]);
    
    MathData dataNew = {Operator, Div};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);
    TreeDeleteNode(tree, tree->nodes[node].branch[Right]);
    TreeDeleteNode(tree, tree->nodes[node].branch[Left]);

    dataNew.type = Number;
    dataNew.value = 1 * PRECISION;

    TreeInsertNode(tree, node, Left, &dataNew);
    TreeGlueSubtree(tree, argCpy, node, Right, argCount);

    free(argCpy);

    return 0;
}

int DiffSqrt(struct _TreeMath* tree, const int node) {
    Node* funcCpy = TreeCopySubtree(tree, node);
    int funcCount = TreeCountSubtree(tree, node);
    
    MathData dataNew = {Operator, Div};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);
    TreeDeleteNode(tree, tree->nodes[node].branch[Right]);
    TreeDeleteNode(tree, tree->nodes[node].branch[Left]);

    dataNew.type = Number;
    dataNew.value = 1 * PRECISION;
    TreeInsertNode(tree, node, Left, &dataNew);

    dataNew.type = Operator;
    dataNew.value = Mul;
    TreeInsertNode(tree, node, Right, &dataNew);

    int branchR = tree->nodes[node].branch[Right];

    dataNew.type = Number;
    dataNew.value = 2 * PRECISION;
    TreeInsertNode(tree, branchR, Left, &dataNew);

    TreeGlueSubtree(tree, funcCpy, branchR, Right, funcCount);

    free(funcCpy);

    return 0;
}

int DiffOperator(struct _TreeMath* tree, const int node) {
    // getting func's derivative. these functions only change function's node, f.ex: sin(nx) -> cos(nx); cos(nx) -> -1 * sin(nx)
    switch (tree->nodes[node].data.value) {
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
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    Node* leftCpy = TreeCopySubtree(tree, branchL);
    int leftCount = TreeCountSubtree(tree, branchL);
    Node* rightCpy = TreeCopySubtree(tree, branchR);
    int rightCount = TreeCountSubtree(tree, branchR);

    TreeDeleteNode(tree, branchL);
    TreeDeleteNode(tree, branchR);

    MathData dataNew = {Operator, Sum};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);

    dataNew.type = Operator;
    dataNew.value = Mul;
    TreeInsertNode(tree, node, Left, &dataNew);
    TreeInsertNode(tree, node, Right, &dataNew);

    branchL = tree->nodes[node].branch[Left];
    branchR = tree->nodes[node].branch[Right];

    TreeGlueSubtree(tree, leftCpy, branchL, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, branchL, Right, rightCount);
    TreeGlueSubtree(tree, leftCpy, branchR, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, branchR, Right, rightCount);

    DiffNode(tree, tree->nodes[branchL].branch[Left]);
    DiffNode(tree, tree->nodes[branchR].branch[Right]);

    free(leftCpy);
    free(rightCpy);

    return 0;
}

int DiffDiv(struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    Node* leftCpy = TreeCopySubtree(tree, branchL);
    int leftCount = TreeCountSubtree(tree, branchL);
    Node* rightCpy = TreeCopySubtree(tree, branchR);
    int rightCount = TreeCountSubtree(tree, branchR);

    TreeDeleteNode(tree, branchL);
    TreeDeleteNode(tree, branchR);

    MathData dataNew = {Operator, Mul};
    TreeInsertNode(tree, node, Right, &dataNew);

    dataNew.type = Operator;
    dataNew.value = Sub;
    TreeInsertNode(tree, node, Left, &dataNew);

    branchL = tree->nodes[node].branch[Left];
    branchR = tree->nodes[node].branch[Right];

    TreeGlueSubtree(tree, rightCpy, branchR, Right, rightCount);
    TreeGlueSubtree(tree, rightCpy, branchR, Left, rightCount);

    dataNew.type = Operator;
    dataNew.value = Mul;
    TreeInsertNode(tree, branchL, Left, &dataNew);
    TreeInsertNode(tree, branchL, Right, &dataNew);

    int branchLL = tree->nodes[branchL].branch[Left];
    int branchLR = tree->nodes[branchL].branch[Right];

    TreeGlueSubtree(tree, leftCpy, branchLL, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, branchLL, Right, rightCount);

    TreeGlueSubtree(tree, leftCpy, branchLR, Left, leftCount);
    TreeGlueSubtree(tree, rightCpy, branchLR, Right, rightCount);

    DiffNode(tree, tree->nodes[branchLL].branch[Left]);
    DiffNode(tree, tree->nodes[branchLR].branch[Right]);

    free(leftCpy);
    free(rightCpy);

    return 0;
}

int DiffSumSub(struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    DiffNode(tree, branchL);
    DiffNode(tree, branchR);

    return 0;
}

int DiffPow(struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    Node* powCpy = TreeCopySubtree(tree, node);
    int powCount = TreeCountSubtree(tree, node);
    Node* leftCpy = TreeCopySubtree(tree, branchL);
    int leftCount = TreeCountSubtree(tree, branchL);
    Node* rightCpy = TreeCopySubtree(tree, branchR);
    int rightCount = TreeCountSubtree(tree, branchR);

    TreeDeleteNode(tree, branchL);
    TreeDeleteNode(tree, branchR);

    MathData dataNew = {Operator, Mul};
    TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew);
    TreeInsertNode(tree, node, Right, &dataNew);
    TreeGlueSubtree(tree, powCpy, node, Left, powCount);

    branchL = tree->nodes[node].branch[Left];
    branchR = tree->nodes[node].branch[Right];

    TreeGlueSubtree(tree, rightCpy, branchR, Right, rightCount);

    dataNew.type = Function;
    dataNew.value = Ln;
    TreeInsertNode(tree, branchR, Left, &dataNew);

    TreeGlueSubtree(tree, leftCpy, tree->nodes[branchR].branch[Left], Left, leftCount);

    DiffNode(tree, branchR);

    free(leftCpy);
    free(rightCpy);
    free(powCpy);

    return 0;
}

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

    dprintf(fd, "\\] \n\\end{document}");

    system("pdflatex output/report.tex");

    return 0;
}

int _DiffPrintTree(const int fd, struct _TreeMath* tree, const int node) {
    if (node != 0) {
        switch (tree->nodes[node].data.type) {
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
    if (tree->nodes[node].data.value == 3140) {
        dprintf(fd, "\\pi");
    } else if (tree->nodes[node].data.value == 2730) {
        dprintf(fd, "e");
    } else if (tree->nodes[node].data.value >= 0) { 
        dprintf(fd, "%0.2f", (double)(tree->nodes[node].data.value) / PRECISION);
    } else {
        dprintf(fd, "(%0.2f)", (double)(tree->nodes[node].data.value) / PRECISION); 
    }
    

    return 0;
}

int PrintOperator(const int fd, struct _TreeMath* tree, const int node) {
    switch (tree->nodes[node].data.value) {
        case Sum: if (PrintSum(fd, tree, node) == -1) {return -1;} break;
        case Sub: if (PrintSub(fd, tree, node) == -1) {return -1;} break;
        case Mul: if (PrintMul(fd, tree, node) == -1) {return -1;} break;
        case Div: if (PrintDiv(fd, tree, node) == -1) {return -1;} break;
        case Pow: if (PrintPow(fd, tree, node) == -1) {return -1;} break;
    }

    return 0;
}

int PrintSum(const int fd, struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    if (_DiffPrintTree(fd, tree, branchL) == -1) {
        return -1;
    }

    dprintf(fd, " + ");

    if (_DiffPrintTree(fd, tree, branchR) == -1) {
        return -1;
    }

    return 0;
}

int PrintSub(const int fd, struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    if (_DiffPrintTree(fd, tree, branchL) == -1) {
        return -1;
    }

    dprintf(fd, " - ");

    if (tree->nodes[branchR].data.type == Operator 
        && (tree->nodes[branchR].data.value == Sub 
        || tree->nodes[branchR].data.value == Sum)) {
        dprintf(fd, " \\left( ");

        if (_DiffPrintTree(fd, tree, branchR) == -1) {
            return -1;
        }

        dprintf(fd, " \\right) ");
    } else {
        if (_DiffPrintTree(fd, tree, branchR) == -1) {
            return -1;
        }
    }

    return 0;
}

int PrintMul(const int fd, struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    if (tree->nodes[branchL].data.type == Operator 
        && (tree->nodes[branchL].data.value == Sub 
        || tree->nodes[branchL].data.value == Sum
        || tree->nodes[branchR].data.value == Div)) {
        dprintf(fd, " \\left( ");

        if (_DiffPrintTree(fd, tree, branchL) == -1) {
            return -1;
        }

        dprintf(fd, " \\right) ");
    } else {
        if (_DiffPrintTree(fd, tree, branchL) == -1) {
            return -1;
        }
    }

    dprintf(fd, " \\cdot ");
    
    if (tree->nodes[branchR].data.type == Operator 
        && (tree->nodes[branchR].data.value == Sub 
        || tree->nodes[branchR].data.value == Sum)) {
        dprintf(fd, " \\left( ");

        if (_DiffPrintTree(fd, tree, branchR) == -1) {
            return -1;
        }

        dprintf(fd, " \\right) ");
    } else {
        if (_DiffPrintTree(fd, tree, branchR) == -1) {
            return -1;
        }
    }

    return 0;
}

int PrintDiv(const int fd, struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    dprintf(fd, "\\dfrac{ ");

    if (_DiffPrintTree(fd, tree, branchL) == -1) {
        return -1;
    }    

    dprintf(fd, " }{ ");

    if (_DiffPrintTree(fd, tree, branchR) == -1) {
        return -1;
    }

    dprintf(fd, " } ");

    return 0;
}

int PrintPow(const int fd, struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];
    int branchR = tree->nodes[node].branch[Right];

    dprintf(fd, " \\left( ");    

    if (_DiffPrintTree(fd, tree, branchL) == -1) {
        return -1;
    }    

    dprintf(fd, " \\right)^{");

    if (_DiffPrintTree(fd, tree, branchR) == -1) {
        return -1;
    }

    dprintf(fd, " } ");

    return 0;
}

int PrintFunction(const int fd, struct _TreeMath* tree, const int node) {
    int branchL = tree->nodes[node].branch[Left];

    switch (tree->nodes[node].data.value) {
        case Sin:   dprintf(fd, "\\sin{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Cos:   dprintf(fd, "\\cos{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Tan:   dprintf(fd, "\\tan{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Ctan:  dprintf(fd, "\\ctan{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Asin:  dprintf(fd, "\\arcsin{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Acos:  dprintf(fd, "\\arccos{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Atan:  dprintf(fd, "\\arctan{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Actan: dprintf(fd, "\\arcctan{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Log:   dprintf(fd, "\\log{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Ln:    dprintf(fd, "\\ln{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
        case Sqrt:  dprintf(fd, "\\sqrt{ \\left("); if (_DiffPrintTree(fd, tree, branchL) == -1) {return -1;} dprintf(fd, "\\right) }"); break; 
    }

    return 0;
}

//####################//
