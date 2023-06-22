#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <stdbool.h>
#include "mpc.h"

// Kovacs value
typedef struct
{
    int type;
    long num;
    int err;
} kval;

enum
{
    KVAL_NUM,
    KVAL_ERR
};

enum
{
    KERR_DIV_ZERO,
    KERR_BAD_OP,
    KERR_BAD_NUM
};

kval eval(mpc_ast_t *t);
kval eval_op(kval x, char *op, kval y);
kval kval_num(long num);
kval kval_err(int errorCode);
void kval_println(kval kv);
void kval_print(kval kv);
void kval_print_error(int errorCode);

int main()
{
    /* Create parsers */
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Kovacs = mpc_new("kovacs");

    /* Define them as a part of the language */
    /*
        Right now, expressions like
        + 1 1 are valid, as are
        + 1 (* 4 4)

        But not (+ 1 1)
    */

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "                                                       \
            number   : /-?[0-9]+/ ;                             \
            operator : '+' | '-' | '*' | '/' ;                  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            kovacs   : /^/ <operator> <expr>+ /$/ ;             \
        ",
        Number, Operator, Expr, Kovacs);

    puts("Kovacs Version 0.0.0.0.1");
    puts("Press Crtl+C to Exit\n");

    while (1)
    {
        // Prompt
        char *input = readline("kovacs> ");

        // Remember Prompts
        add_history(input);

        // Init parsing result
        mpc_result_t result;

        // Parse input into result
        // If successful, mpc_parse returns 1
        if (mpc_parse("<stdin>", input, Kovacs, &result)) // Passes r via memory address to be written directly to
        {
            // Print parsed results
            mpc_ast_print(result.output);
            printf("\n\n");

            kval evaluatedOutput = eval(result.output);
            kval_println(evaluatedOutput);

            printf("\n\n");
            mpc_ast_delete(result.output);
        }
        else
        {
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Kovacs);
    return 0;
}

// Evaluate an abstract syntax tree of type mpc_ast_t
kval eval(mpc_ast_t *ast)
{
    /*
        strstr takes two char* pointers and returns a pointer or 0

        If r = 0, the second string is not a substring.
        If r != 0, r is a pointer to the location of the substring
    */
    // Base case: If this tag contains a number, convert the string contents to an integer
    if (strstr(ast->tag, "number"))
    {
        // errno is from <errno.h>, a header that provides error codes and their constants
        errno = 0;
        long x = strtol(ast->contents, NULL, 10);

        bool x_out_of_range = errno == ERANGE;

        return x_out_of_range
                   ? kval_err(KERR_BAD_NUM)
                   : kval_num(x);
    }

    // Operators are always second children
    char *op = ast->children[1]->contents;

    // The 3rd child is the first child being operated on
    kval x = eval(ast->children[2]);

    int i = 3;
    while (strstr(ast->children[i]->tag, "expr"))
    {
        x = eval_op(x, op, eval(ast->children[i]));
        i++;
    }

    return x;
}

kval eval_op(kval x, char *op, kval y)
{
    if (x.type == KVAL_ERR || y.type == KVAL_ERR)
    {
        return (x.type == KVAL_ERR) ? x : y;
    }

    long x_val = x.num;
    long y_val = y.num;
    long op_result;

    switch (*op)
    {
    case '+':
        op_result = x_val + y_val;
        break;
    case '-':
        op_result = x_val - y_val;
        break;
    case '*':
        op_result = x_val * y_val;
        break;
    case '/':
        if (y_val == 0)
        {
            return kval_err(KERR_DIV_ZERO);
        }

        op_result = x_val / y_val;
        break;
    default:
        return kval_err(KERR_BAD_OP);
    }

    return kval_num(op_result);
}

kval kval_num(long num)
{
    kval kv;

    kv.type = KVAL_NUM;
    kv.num = num;

    return kv;
}

kval kval_err(int errorCode)
{
    kval kv;

    kv.type = KVAL_ERR;
    kv.err = errorCode;

    return kv;
}

void kval_println(kval kv)
{
    kval_print(kv);
    putchar('\n');
}

void kval_print(kval kv)
{
    switch (kv.type)
    {
    case KVAL_NUM:
        printf("%li", kv.num);
        return;
    case KVAL_ERR:
        kval_print_error(kv.err);
        return;
    }
}

void kval_print_error(int errorCode)
{
    switch (errorCode)
    {
    case KERR_BAD_NUM:
        printf("Error: Invalid number");
        return;
    case KERR_BAD_OP:
        printf("Error: Invalid operation");
        return;
    case KERR_DIV_ZERO:
        printf("Error: Division by zero");
        return;
    default:
        printf("Error: Unknown");
        return;
    }
}
