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

//####################//

#ifndef DIFF_H_INCLUDED
#define DIFF_H_INCLUDED

//####################//

#include "tree.h"

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

//####################//

//####################//

#endif

