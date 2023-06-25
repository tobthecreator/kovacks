#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <editline/readline.h>

#include "mpc.h"
#include "kval.h"
#include "types.h"
#include "kenv.h"
#include "quotes.h"
#include "parser.h"
#include "builtin.h"

int main(int argc, char **argv)
{
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

    puts("Kovacs Version 0.1");
    print_altered_carbon_quote();
    puts("Press Crtl+C to Exit\n");

    // Build REPL Environment
    kenv *e = kenv_init();
    kenv_add_builtins(e);

    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }

    char *stdlib_filepath = strcat(cwd, "/stdlib.k");

    kval *standard_libararies = kval_add(kval_sexpr(), kval_str(stdlib_filepath));
    builtin_load(e, standard_libararies);

    // If there is only one argument, open the REPL
    if (argc == 1)
    {
        while (1)
        {
            // Prompt
            char *input = readline("kovacs> ");

            // Give the REPL history, allowing a better UX
            add_history(input);

            // Parse input into result
            mpc_result_t result;
            if (mpc_parse("<stdin>", input, Kovacs, &result)) // Passes r via memory address to be written directly to
            {
                // Uncomment below if you want to print parser results
                // mpc_ast_print(result.output);

                kval *x = kval_eval(e, kval_read(result.output));
                kval_println(x);
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

    // If there is more than one argument, it's probably files to run
    if (argc >= 2)
    {
        for (int i = 1; i < argc; i++)
        {

            kval *file_arguments = kval_add(kval_sexpr(), kval_str(argv[i]));

            kval *x = builtin_load(e, file_arguments);
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
