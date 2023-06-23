#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <stdbool.h>
#include "mpc.h"
#include "kval.h"
#include "types.h"
#include "kenv.h"
#include "quotes.h"

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
            symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        \
            sexpr   : '(' <expr>* ')' ;                         \
            qexpr   : '{' <expr>* '}' ;                         \
            expr    : <number> | <symbol> | <sexpr> | <qexpr> ; \
            kovacs  : /^/ <expr>+ /$/ ;                         \
        ",
        Number, Symbol, Sexpr, Qexpr, Expr, Kovacs);

    puts("Kovacs Version 0.0.0.0.1");
    print_altered_carbon_quote();
    puts("Press Crtl+C to Exit\n");

    // Build REPL Environment
    kenv *e = kenv_init();
    kenv_add_builtins(e);

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

            kval *x = kval_eval(e, kval_read(result.output));
            kval_println(x);

            kval_del(x);

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

    kenv_del(e);

    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Kovacs);
    return 0;
}
