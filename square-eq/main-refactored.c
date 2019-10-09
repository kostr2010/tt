#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <float.h>


enum Type{LINEAR,
          DEGENERATE, 
          SQUARE, 
          IDENTITY, 
          INSOLUBLE, 
          SQUARE_FULL, 
          SQUARE_IMAGINARY, 
          SQUARE_REAL};

struct Complex {
    double re;
    double im;
};
typedef struct Complex Complex;

struct Solution {
    Complex x1;
    Complex x2;
    enum Type type;
};
typedef struct Solution Solution;

struct Coeffs {
    double a;
    double b;
    double c;
};
typedef struct Coeffs Coeffs;

/**
 * ReadCoeffs function fills the fields  of the Coeffs structure,
 * assures that all 3 fields are filled correctly
 * 
 * @param Coeffs* coeffs    - structure of 3 doubles;
 * 
 * @return                  - nothing.
 */
void ReadCoeffs(Coeffs* coeffs);

/**
 * SolveEquation function solves <ax^2 + bx + c> equation, 
 * where a, b, c are the fields of Coeffs coeffs structure.
 * Answer is written down to the Solution sol structure.
 * 
 * @param Coeffs coeffs - structure of coefficients of the equation;
 * @param Solution* sol - structure to write out answer to;
 * 
 * @return              - nothing. 
 */
void SolveEquation(Coeffs coeffs, Solution* sol);

/**
 * _GetEquationType is an internal function, designed only to be used inside
 * the SolveEquation function. It determines type of equation, judging by the 
 * coefficients of the equation.
 * 
 * @param Coeffs coeffs - structure of coefficients of the equation;
 * @param Solution* sol - structure to write down type to;
 * 
 * @return              - nothing.
 */
void _GetEquationType(Coeffs coeffs, Solution* sol);

/**
 * _SolveLinear is an internal function, designed only to be used inside
 * the SolveEquation function. It solves linear equation with given coefficients
 * and writes out the solution to sol variable.
 * 
 * @param double a      - coefficient of x in <ax + c = 0> equation;
 * @param double b      - free coefficient in <ax + c = 0> equation;
 * @param double* sol   - variable to write out answer to;
 * 
 * @return              - nothing.
 */
void _SolveLinear(double a, double b, double* sol);

/**
 *_SolveSquare is an internal function, designed only to be used inside
 * the SolveEquation function. It solves square equation with given coefficients
 * and writes out the solution to sol variable.
 * 
 * @param Coeffs coeffs - structure of coefficients of the equation;
 * @param Solution* sol - structure to write down answer to;
 * 
 * @return              - nothing
 */
void _SolveSquare(Coeffs coeffs, Solution* sol);

/**
 * PrintSolution prints formatted solution of square equation.
 * 
 * @param Solution* sol - variable to get answer from;
 * 
 * @return              - nothing.
 */
void PrintSolution(Solution* sol);


int main() {
    Coeffs coeffs = {NAN, NAN, NAN};
    Solution sol = {{NAN, NAN, 0}, {NAN, NAN, 0}};

    printf("############################################\n"
           "#This programm solves given square equation#\n"
           "############################################\n\n");

    ReadCoeffs(&coeffs);
    SolveEquation(coeffs, &sol);
    PrintSolution(&sol);

    return 0;
}

void ReadCoeffs(Coeffs* coeffs) {
    assert(coeffs);

    printf("Enter the coefficients of <ax^2 + bx + c = 0> equation in the following format:\n"
           "a b c\n");
    while (scanf("%lf %lf %lf", &(coeffs->a), &(coeffs->b), &(coeffs->c)) != 3) {
        printf("\nIncorrect input. Please, try again.\n"
               "Note: coefficients a, b, c should be Reals, and separation symbol should be '.'\n");
        while(getchar() != '\n');
    }

    printf("\nInput interpretation:\n"
           "a = %lf;\n"
           "b = %lf;\n"
           "c = %lf.\n\n", coeffs->a, coeffs->b, coeffs->c);
}

void SolveEquation(Coeffs coeffs, Solution* sol) {
    assert(isfinite(coeffs.a));
    assert(isfinite(coeffs.b));
    assert(isfinite(coeffs.c));
    assert(sol);

    _GetEquationType(coeffs, sol);

    switch (sol->type) {
    case LINEAR:
        _SolveLinear(coeffs.b, coeffs.c, &(sol->x1.re));
        break;
    case DEGENERATE:
        sol->x2.re = 0;
        _SolveLinear(coeffs.a, coeffs.b, &(sol->x1.re));
        break;
    case SQUARE:
        _SolveSquare(coeffs, sol);
        break;
    default:
        break;
    }
}

void _GetEquationType(Coeffs coeffs, Solution* sol) {
    assert(isfinite(coeffs.a));
    assert(isfinite(coeffs.b));
    assert(isfinite(coeffs.c));
    assert(sol);

    if (coeffs.a == 0 && coeffs.b == 0 && coeffs.c == 0)
        sol->type = IDENTITY;
    else if (coeffs.a == 0 && coeffs.b == 0 && coeffs.c != 0)
        sol->type = INSOLUBLE;
    else if (coeffs.a == 0 && coeffs.b != 0)
        sol->type = LINEAR;
    else if (coeffs.a != 0 && coeffs.c == 0)
        sol->type = DEGENERATE;
    else 
        sol->type = SQUARE;    
}

void _SolveLinear(double a, double b, double* sol) {
    assert(isfinite(a));
    assert(isfinite(b));
    assert(sol);

    *sol = -b / a;
}

void _SolveSquare(Coeffs coeffs, Solution* sol) {
    assert(isfinite(coeffs.a));
    assert(isfinite(coeffs.b));
    assert(isfinite(coeffs.c));
    assert(sol);

    double discr = coeffs.b * coeffs.b - 4 * coeffs.a * coeffs.c;
    double discrErr = 2 * pow(10, -DBL_DIG)
                        * (fabs(coeffs.b) + fabs(2 * coeffs.a) + fabs(2 * coeffs.c));

    if (discr > 0 && fabs(discr) > discrErr) {
        sol->x1.re = (-coeffs.b + sqrt(discr)) / (2 * coeffs.a);
        sol->x2.re = -(sol->x1.re) - coeffs.b/coeffs.a;
        sol->type = SQUARE_REAL;
    } else if (discr < 0 && fabs(discr) > discrErr) {
        sol->x1.re = -coeffs.b / (2 * coeffs.a);
        sol->x1.im = sqrt(fabs(discr)) / (2 * coeffs.a);
        sol->x2.re = sol->x1.re;
        sol->x2.im = -sol->x1.im;
        sol->type = SQUARE_IMAGINARY;
    } else {
        sol->x1.re = -coeffs.b / (2 * coeffs.a);
        sol->type = SQUARE_FULL;
    }
}

void PrintSolution(Solution* sol) {
    assert(sol);

    switch (sol->type) {
    case LINEAR:
    case SQUARE_FULL:
        printf("Solution for this equation is:\n"
               "x1: %lf.\n", sol->x1.re);
        break;
    case DEGENERATE:
    case SQUARE_REAL:
        printf("Solutions for this equation are:\n"
               "x1: %lf;\n"
               "x2: %lf.\n", sol->x1.re, sol->x2.re);
        break;
    case SQUARE_IMAGINARY:
        printf("Solutions for this equation are:\n"
               "x1: %lf + (%lf)i;\n"
               "x1: %lf + (%lf)i.\n", sol->x1.re, sol->x1.im, sol->x2.re, sol->x2.im);
        break;
    case IDENTITY:
        printf("Solution is R set.\n");
        break;
    case INSOLUBLE:
        printf("There are no solutions for this equation.\n");
        break;
    default:
        printf("Fuck me, fuck my life.\n");
        break;
    }
}