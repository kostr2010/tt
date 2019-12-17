#ifndef DIFF_H_INCLUDED
#define DIFF_H_INCLUDED

//####################//

#include "tree.h"

//####################//
// DSL lang

#ifndef DSL_H
#define DSL_H

#define R  tree->nodes[node].branch[Right]
#define L  tree->nodes[node].branch[Left]
#define RR tree->nodes[R].branch[Right]
#define RL tree->nodes[R].branch[Left]
#define LR tree->nodes[L].branch[Right]
#define LL tree->nodes[L].branch[Left]

#define TYPE(node) tree->nodes[node].data.type
#define VALUE(node) tree->nodes[node].data.value

#define DELETE(node)                            TreeDeleteNode(tree, node)

#define CHANGE(node, typeNew, valueNew)         dataNew.type = typeNew;\
                                                dataNew.value = valueNew;\
                                                TreeChangeNode(tree, node, NULL, NULL, NULL, &dataNew)

#define INSERT(node, branch, typeNew, valueNew) dataNew.type = typeNew;\
                                                dataNew.value = valueNew;\
                                                TreeInsertNode(tree, node, branch, &dataNew)

#define D(node)                                 DiffNode(tree, node)

#endif

//####################//

enum MathTypes {
    Variable,
    Number,
    Operator,
    Function,
};

enum Operators {
    Mul,
    Div,
    Sum,
    Sub,
    Pow,
};

enum Functions {
    Sin,
    Cos,
    Tan,
    Ctan,
    Asin,
    Acos,
    Atan,
    Actan,
    Ln,
    Log,
    Sqrt,
};

//####################//

struct _TreeMath* DiffReadExpression(const char* pathname);
    int GetTier4(char** s, struct _TreeMath* tree, int node);
        int GetGr(char** s, struct _TreeMath* tree, int node);
    int GetTier3(char** s, struct _TreeMath* tree, int node, int branch);
        int GetSumSub(char** s, struct _TreeMath* tree, int node, int branch);
    int GetTier2(char** s, struct _TreeMath* tree, int node, int branch);
        int GetMulDiv(char** s, struct _TreeMath* tree, int node, int branch);
    int GetTier1(char** s, struct _TreeMath* tree, int node, int branch);
        int GetPow(char** s, struct _TreeMath* tree, int node, int branch);
    int GetTier0(char** s, struct _TreeMath* tree, int node, int branch);
        int GetVar(char** s, struct _TreeMath* tree, int node, int branch);
        int GetNum(char** s, struct _TreeMath* tree, int node, int branch);
        int GetPar(char** s, struct _TreeMath* tree, int node, int branch);
        int GetFunc(char** s, struct _TreeMath* tree, int node, int branch);

int DiffGetDerivative(struct _TreeMath* tree);
    int DiffNode(struct _TreeMath* tree, const int node);
        int DiffVariable(struct _TreeMath* tree, const int node);
        int DiffNumber(struct _TreeMath* tree, const int node);
        int DiffFunction(struct _TreeMath* tree, const int node);
            int DiffSin(struct _TreeMath* tree, const int node);
            int DiffCos(struct _TreeMath* tree, const int node);
            int DiffTan(struct _TreeMath* tree, const int node);
            int DiffCtan(struct _TreeMath* tree, const int node);
            int DiffAsin(struct _TreeMath* tree, const int node);
            int DiffAcos(struct _TreeMath* tree, const int node);
            int DiffAtan(struct _TreeMath* tree, const int node);
            int DiffActan(struct _TreeMath* tree, const int node);
            int DiffLog(struct _TreeMath* tree, const int node);
            int DiffLn(struct _TreeMath* tree, const int node);
            int DiffSqrt(struct _TreeMath* tree, const int node);
        int DiffOperator(struct _TreeMath* tree, const int node);
            int DiffMul(struct _TreeMath* tree, const int node);
            int DiffDiv(struct _TreeMath* tree, const int node);
            int DiffSumSub(struct _TreeMath* tree, const int node);
            int DiffPow(struct _TreeMath* tree, const int node);

int DiffSimplify(struct _TreeMath* tree);
    int _DiffSimplify(struct _TreeMath* tree, const int node, int* flag);
        int SimplifyOperator(struct _TreeMath* tree, const int node, int* flag);
            int SimplifyMul(struct _TreeMath* tree, const int node, int* flag);
            int SimplifyDiv(struct _TreeMath* tree, const int node, int* flag);
            int SimplifySum(struct _TreeMath* tree, const int node, int* flag);         
            int SimplifySub(struct _TreeMath* tree, const int node, int* flag);
        int SimplifyFunction(struct _TreeMath* tree, const int node, int* flag);
            int SimplifySin(struct _TreeMath* tree, const int node, int* flag);
            int SimplifyCos(struct _TreeMath* tree, const int node, int* flag);
            int SimplifyTan(struct _TreeMath* tree, const int node, int* flag);

int DiffPrintTree(struct _TreeMath* tree);
int _DiffPrintTree(int fd, struct _TreeMath* tree, int node);
    int PrintVariable(const int fd, struct _TreeMath* tree, const int node);
    int PrintNumber(const int fd, struct _TreeMath* tree, const int node);
    int PrintOperator(const int fd, struct _TreeMath* tree, const int node);
        int PrintSum(const int fd, struct _TreeMath* tree, const int node);
        int PrintSub(const int fd, struct _TreeMath* tree, const int node);
        int PrintMul(const int fd, struct _TreeMath* tree, const int node);
        int PrintDiv(const int fd, struct _TreeMath* tree, const int node);
        int PrintPow(const int fd, struct _TreeMath* tree, const int node);
    int PrintFunction(const int fd, struct _TreeMath* tree, const int node);    

//####################//

#endif
