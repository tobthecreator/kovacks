#include <stdlib.h>
#include <stdbool.h>
#include "kval.h"
#include "errors.h"
#include "builtin.h"
#include "types.h"
#include "kenv.h"

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

kval *kval_fun(kbuiltin func)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_FUN;
    kv->fun = func;

    return kv;
}

kval *kval_eval(kenv *e, kval *kv)
{
    printf("kval_eval\n");

    if (kv->type == KVAL_SYM)
    {
        printf("kval_eval.is_kval_sym\n");
        kval *x = kenv_get(e, kv);
        kval_del(kv);
        return x;
    }

    if (kv->type == KVAL_SEXPR)
    {
        printf("kval_eval.KVAL_SEXPR\n");
        return kval_eval_sexpr(e, kv);
    }

    return kv;
}

kval *kval_eval_sexpr(kenv *e, kval *kv)
{
    // Evaluate cells
    for (int i = 0; i < kv->count; i++)
    {
        kv->cells[i] = kval_eval(e, kv->cells[i]);
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

    kval *f = kval_pop(kv, 0);
    if (f->type != KVAL_FUN)
    {
        kval_del(f);
        kval_del(kv);

        // TODO: proper error code here
        return kval_err("First child is not a function");
    }

    kval *result = f->fun(e, kv);
    kval_del(f);

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

    case KVAL_FUN:
        printf("<function>");
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
    case KVAL_FUN:
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

    /* If root (>), qexpr or sexpr then create empty list */
    kval *x = NULL;
    if (strcmp(ast->tag, ">") == 0)
    {
        x = kval_sexpr();
    }

    if (strstr(ast->tag, "sexpr"))
    {
        x = kval_sexpr();
    }

    if (strstr(ast->tag, "qexpr"))
    {
        x = kval_qexpr();
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

kval *kval_join(kval *x, kval *y)
{

    /* For each cell in 'y' add it to 'x' */
    while (y->count)
    {
        x = kval_add(x, kval_pop(y, 0));
    }

    /* Delete the empty 'y' and return 'x' */
    kval_del(y);
    return x;
}

kval *kval_copy(kval *v)
{

    kval *x = malloc(sizeof(kval));
    x->type = v->type;

    switch (v->type)
    {

    /* Copy functions and numbers directly */
    case KVAL_FUN:
        x->fun = v->fun;
        break;

    case KVAL_NUM:
        x->num = v->num;
        break;

    /* Copy strings using malloc and strcpy */
    case KVAL_ERR:
        x->err = malloc(strlen(v->err) + 1);
        strcpy(x->err, v->err);
        break;

    case KVAL_SYM:
        x->sym = malloc(strlen(v->sym) + 1);
        strcpy(x->sym, v->sym);
        break;

    /* Copy lists by copying each sub-expression */
    case KVAL_SEXPR:
    case KVAL_QEXPR:
        x->count = v->count;
        x->cells = malloc(sizeof(kval *) * x->count);
        for (int i = 0; i < x->count; i++)
        {
            x->cells[i] = kval_copy(v->cells[i]);
        }
        break;
    }

    return x;
}
