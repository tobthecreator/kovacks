#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <stdbool.h>
#include "mpc.h"
#include "kval.h"
#include "types.h"
#include "kenv.h"
#include "quotes.h"
#include "parser.h"
#include "builtin.h"

int main(int argc, char **argv)
{
    /* Create parsers */
    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    Sexpr = mpc_new("sexpr");
    Expr = mpc_new("expr");
    Kovacs = mpc_new("kovacs");
    Qexpr = mpc_new("qexpr");
    String = mpc_new("string");
    Comment = mpc_new("comment");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "                                                                               \
            number  : /-?[0-9]+/ ;                                                      \
            symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                                \
            sexpr   : '(' <expr>* ')' ;                                                 \
            qexpr   : '{' <expr>* '}' ;                                                 \
            expr    : <number> | <symbol> | <sexpr> | <qexpr> | <string> | <comment>;   \
            kovacs  : /^/ <expr>+ /$/ ;                                                 \
            string  : /\"(\\\\.|[^\"])*\"/ ;                                            \
            comment : /;[^\\r\\n]*/ ;                                                   \
        ",
        Number, Symbol, Sexpr, Qexpr, Expr, Kovacs, String, Comment);

    puts("Kovacs Version 0.0.0.0.1");
    print_altered_carbon_quote();
    puts("Press Crtl+C to Exit\n");

    // Build REPL Environment
    kenv *e = kenv_init();
    kenv_add_builtins(e);

    kval *standard_libararies = kval_add(kval_sexpr(), kval_str("stdlib.k"));
    builtin_load(e, standard_libararies);

    if (argc == 1)
    {
        while (1)
        {
            // Prompt
            char *input = readline("kovacs> ");

            // Remember prompts
            add_history(input);

            // Init parsing result
            mpc_result_t result;

            // Parse input into result
            // If successful, mpc_parse returns 1
            if (mpc_parse("<stdin>", input, Kovacs, &result)) // Passes r via memory address to be written directly to
            {
                // Print parsed results
                // mpc_ast_print(result.output);
                // printf("\n\n");

                kval *x = kval_eval(e, kval_read(result.output));
                kval_println(x);

                // printf("\n\n");
                kval_del(x);
                mpc_ast_delete(result.output);
            }
            else
            {
                mpc_err_print(result.error);
                mpc_err_delete(result.error);
            }

            free(input);
        }
    }

    // If given files
    if (argc >= 2)
    {

        for (int i = 1; i < argc; i++)
        {

            /* Argument list with a single argument, the filename */
            kval *args = kval_add(kval_sexpr(), kval_str(argv[i]));

            /* Pass to builtin load and get the result */
            kval *x = builtin_load(e, args);

            /* If the result is an error be sure to print it */
            if (x->type == KVAL_ERR)
            {
                kval_println(x);
            }

            kval_del(x);
        }
    }

    kenv_del(e);
    mpc_cleanup(8, Number, Symbol, Sexpr, Qexpr, Expr, Kovacs, String, Comment);

    return 0;
}
