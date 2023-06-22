#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"

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
            kovacs    : /^/ <operator> <expr>+ /$/ ;            \
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
            mpc_ast_print(result.output);
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