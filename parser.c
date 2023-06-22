#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"

long eval(mpc_ast_t *t);
long eval_op(long x, char *op, long y);

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

            long evaluatedOutput = eval(result.output);
            printf("%li\n", evaluatedOutput);

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
long eval(mpc_ast_t *ast)
{
    /*
        strstr takes two char* pointers and returns a pointer or 0

        If r = 0, the second string is not a substring.
        If r != 0, r is a pointer to the location of the substring
    */
    // If this tags contains a number, convert the string contents to an integer
    if (strstr(ast->tag, "number"))
    {
        return atoi(ast->contents);
    }

    // Operators are always second children
    char *op = ast->children[1]->contents;

    // The 3rd child is the first child being operated on
    long x = eval(ast->children[2]);

    int i = 3;
    while (strstr(ast->children[i]->tag, "expr"))
    {
        x = eval_op(x, op, eval(ast->children[i]));
        i++;
    }

    return x;
}

long eval_op(long x, char *op, long y)
{
    switch (*op)
    {
    case '+':
        return x + y;

    case '-':
        return x - y;

    case '*':
        return x * y;

    case '/':
        return x / y;

    default:
        return 0;
    }

    return 0;
}
