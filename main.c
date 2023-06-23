#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <stdbool.h>
#include "mpc.h"
#include "kval.h"

int main()
{
    /* Create parsers */
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Kovacs = mpc_new("kovacs");
    mpc_parser_t *Qexpr = mpc_new("qexpr");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "                                                       \
            number  : /-?[0-9]+/ ;                              \
            symbol  : '+' | '-' | '*' | '/' | \"tail\"          \
                    | \"list\" | \"head\" | \"join\"            \
                    | \"eval\" ;                                \
            sexpr   : '(' <expr>* ')' ;                         \
            qexpr   : '{' <expr>* '}' ;                         \
            expr    : <number> | <symbol> | <sexpr> | <qexpr> ; \
            kovacs  : /^/ <expr>+ /$/ ;                         \
        ",
        Number, Symbol, Sexpr, Qexpr, Expr, Kovacs);

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

            kval *x = kval_eval(kval_read(result.output));
            kval_println(x);

            // kval *y = kval_eval(x);
            // kval_println(y);

            kval_del(x);
            // kval_del(y);

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

    mpc_cleanup(4, Number, Symbol, Sexpr, Qexpr, Expr, Kovacs);
    return 0;
}
