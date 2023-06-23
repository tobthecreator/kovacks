#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <stdbool.h>
#include "mpc.h"

// Kovacs value
typedef struct kval
{
    int type;
    long num;

    char *err;
    char *sym;

    int count;
    struct kval **cells;
} kval;

#define KERR_DIV_ZERO "Division by zero"
#define KERR_BAD_OP "Invalid operation"
#define KERR_BAD_NUM "Invalid number"
#define KERR_UNSUPPORTED_TYPE "Unsupported type"
#define KERR_BAD_SEXPR "Invalid S-expression"
#define KERR_UNKNOWN "Unknown"

enum
{
    KVAL_NUM,
    KVAL_ERR,
    KVAL_SYM,
    KVAL_SEXPR,
    KVAL_QEXPR
};

kval eval(mpc_ast_t *t);
kval eval_op(kval x, char *op, kval y);
kval *kval_num(long num);
kval *kval_err(char *err);
kval *kval_sym(char *s);
kval *kval_sexpr(void);

void kval_del(kval *kv);
kval *kval_read(mpc_ast_t *ast);
kval *kval_add(kval *kv, kval *new_cell);
kval *kval_read_num(mpc_ast_t *ast);
void kval_println(kval *kv);
void kval_print(kval *kv);
void kval_expr_print(kval *kv, char open, char close);

kval *kval_eval_sexpr(kval *kv);
kval *kval_eval(kval *kv);
kval *kval_pop(kval *kv, int i);
kval *kval_take(kval *v, int i);
kval *builtin_op(kval *kv, char *op);

int main()
{
    /* Create parsers */
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Kovacs = mpc_new("kovacs");
    mpc_parser_t *Qexpr = mpc_new("qexpr");

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

kval *kval_eval(kval *kv)
{
    if (kv->type == KVAL_SEXPR)
    {
        return kval_eval_sexpr(kv);
    }

    return kv;
}

kval *kval_eval_sexpr(kval *kv)
{
    // Evaluate cells
    for (int i = 0; i < kv->count; i++)
    {
        kv->cells[i] = kval_eval(kv->cells[i]);
    }

    // Check for errors
    for (int i = 0; i < kv->count; i++)
    {
        bool is_error = kv->cells[i]->type == KVAL_ERR;

        if (is_error)
        {
            return kval_take(kv, i);
        }
    }

    // Base case: No cells
    if (kv->count == 0)
    {
        return kv;
    }

    // Base case: One cell
    if (kv->count == 1)
    {
        return kval_take(kv, 0);
    }

    kval *cell0 = kval_pop(kv, 0);
    if (cell0->type != KVAL_SYM)
    {
        kval_del(cell0);
        kval_del(kv);

        return kval_err(KERR_BAD_SEXPR);
    }

    kval *result = builtin_op(kv, cell0->sym);
    kval_del(cell0);

    return result;
}

kval *kval_pop(kval *kv, int i)
{
    kval *x = kv->cells[i];

    /* Shift memory after the item at "i" over the top */
    memmove(
        &kv->cells[i],
        &kv->cells[i + 1],
        sizeof(kval *) * (kv->count - i - 1));

    kv->count--;

    /* Reallocate memory */
    kv->cells = realloc(kv->cells, sizeof(kval *) * kv->count);

    return x;
}

kval *kval_take(kval *v, int i)
{
    kval *x = kval_pop(v, i);
    kval_del(v);
    return x;
}

kval *builtin_op(kval *kv, char *op)
{
    /* Ensure all arguments are numbers */
    for (int i = 0; i < kv->count; i++)
    {
        if (kv->cells[i]->type != KVAL_NUM)
        {
            kval_del(kv);
            return kval_err(KERR_UNSUPPORTED_TYPE);
        }
    }

    kval *x = kval_pop(kv, 0);

    // If (- 10) -> -10
    bool is_negation = kv->count == 0 && (strcmp(op, "-") == 0);
    if (is_negation)
    {
        x->num *= -1;
    }

    while (kv->count > 0)
    {
        // This is the decrementor
        kval *y = kval_pop(kv, 0);

        switch (*op)
        {
        case '+':
            x->num += y->num;
            break;
        case '-':
            x->num -= y->num;
            break;
        case '*':
            x->num *= y->num;
            break;
        case '/':
            if (y->num == 0)
            {
                return kval_err(KERR_DIV_ZERO);
            }

            x->num /= y->num;
            break;

        default:
            kval_del(x);
            kval_del(y);
            x = kval_err(KERR_BAD_OP);
            break;
        }

        kval_del(y);
    }

    kval_del(kv);
    return x;
}

kval *kval_num(long num)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_NUM;
    kv->num = num;

    return kv;
}

kval *kval_sym(char *s)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_SYM;
    kv->sym = malloc(strlen(s) + 1); // REMINDER: strings are null-terminated with '\0'
    strcpy(kv->sym, s);

    return kv;
}

kval *kval_sexpr(void)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_SEXPR;
    kv->count = 0;
    kv->cells = NULL;

    return kv;
}

kval *kval_qexpr(void)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_QEXPR;
    kv->count = 0;
    kv->cells = NULL;

    return kv;
}

kval *kval_err(char *errorMsg)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_ERR;
    kv->err = malloc(strlen(errorMsg + 1));
    strcpy(kv->err, errorMsg);

    return kv;
}

void kval_println(kval *kv)
{
    printf("kval_println: ");
    kval_print(kv);
    putchar('\n');
}

void kval_print(kval *kv)
{
    switch (kv->type)
    {
    case KVAL_NUM:
        printf("%li", kv->num);
        break;
    case KVAL_ERR:
        printf("Error: %s", kv->err);
        break;
    case KVAL_SYM:
        printf("%s", kv->sym);
        break;
    case KVAL_SEXPR:
        kval_expr_print(kv, '(', ')');
        break;

    case KVAL_QEXPR:
        kval_expr_print(kv, '{', '}');
        break;
    }
}

void kval_expr_print(kval *kv, char open, char close)
{
    putchar(open);
    for (int i = 0; i < kv->count; i++)
    {

        kval_print(kv->cells[i]);

        if (i != (kv->count - 1))
        {
            putchar(' ');
        }
    }
    putchar(close);
}

// void kval_print_error(int errorCode)
// {
//     switch (errorCode)
//     {
//     case KERR_BAD_NUM:
//         printf("Error: Invalid number");
//         return;
//     case KERR_BAD_OP:
//         printf("Error: Invalid operation");
//         return;
//     case KERR_DIV_ZERO:
//         printf("Error: Division by zero");
//         return;
//     default:
//         printf("Error: Unknown");
//         return;
//     }
// }

void kval_del(kval *kv)
{
    switch (kv->type)
    {

    /* For Err or Sym free the string data */
    case KVAL_ERR:
        free(kv->err);
        break;

    case KVAL_SYM:
        free(kv->sym);
        break;

    /* Delete all elements inside */
    case KVAL_SEXPR:
    case KVAL_QEXPR:
        for (int i = 0; i < kv->count; i++)
        {
            kval_del(kv->cells[i]);
        }

        /* Also free the memory allocated to contain the pointers */
        free(kv->cells);
        break;

    case KVAL_NUM:
    default:
        break;
    }

    /* Free the memory allocated for the "kval" struct itself */
    free(kv);
}

kval *kval_read(mpc_ast_t *ast)
{
    /* If Symbol or Number return conversion to that type */
    if (strstr(ast->tag, "number"))
    {
        return kval_read_num(ast);
    }
    if (strstr(ast->tag, "symbol"))
    {
        return kval_sym(ast->contents);
    }

    /* If root (>) or sexpr then create empty list */
    kval *x = NULL;
    if (strcmp(ast->tag, ">") == 0)
    {
        x = kval_sexpr();
    }
    if (strstr(ast->tag, "sexpr"))
    {
        x = kval_sexpr();
    }

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < ast->children_num; i++)
    {
        if (
            strcmp(ast->children[i]->contents, "(") == 0 ||
            strcmp(ast->children[i]->contents, ")") == 0 ||
            strcmp(ast->children[i]->contents, "{") == 0 ||
            strcmp(ast->children[i]->contents, "}") == 0 ||
            strcmp(ast->children[i]->tag, "regex") == 0)
        {
            continue;
        }

        x = kval_add(x, kval_read(ast->children[i]));
    }

    return x;
}

kval *kval_add(kval *kv, kval *new_cell)
{
    kv->count++;
    kv->cells = realloc(kv->cells, sizeof(kval *) * kv->count);
    kv->cells[kv->count - 1] = new_cell;

    return kv;
}

kval *kval_read_num(mpc_ast_t *ast)
{
    // errno is from <errno.h>, a header that provides error codes and their constants
    errno = 0;
    long x = strtol(ast->contents, NULL, 10);

    bool x_out_of_range = errno == ERANGE;

    return x_out_of_range
               ? kval_err(KERR_BAD_NUM)
               : kval_num(x);
}
