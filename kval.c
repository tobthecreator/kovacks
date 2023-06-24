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

kval *kval_err(char *errorMsg, ...)
{
    kval *v = malloc(sizeof(kval));
    v->type = KVAL_ERR;

    va_list va;
    va_start(va, errorMsg);

    v->err = malloc(512);

    vsnprintf(v->err, 511, errorMsg, va);

    v->err = realloc(v->err, strlen(v->err) + 1);

    va_end(va);

    return v;
}

kval *kval_fun(kbuiltin func)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_FUN;
    kv->fun = func;

    return kv;
}

kval *kval_lambda(kval *formals, kval *body)
{
    kval *kv = malloc(sizeof(kval));

    kv->type = KVAL_FUN;

    kv->fun = NULL;
    kv->fenv = kenv_init();

    kv->formals = formals;
    kv->body = body;

    return kv;
}

kval *kval_eval(kenv *e, kval *kv)
{
    if (kv->type == KVAL_SYM)
    {
        kval *x = kenv_get(e, kv);
        kval_del(kv);
        return x;
    }

    if (kv->type == KVAL_SEXPR)
    {
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
        kval *err = kval_err(
            "S-Expression starts with incorrect type. "
            "Got %s, Expected %s.",
            ktype_name(f->type), ktype_name(KVAL_FUN));

        kval_del(f);
        kval_del(kv);

        return err;
    }

    kval *result = kval_call(e, f, kv);
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

    case KVAL_STR:
        kval_print_str(kv);
        break;

    case KVAL_FUN:
        if (kv->fun)
        {
            printf("<builtin>");
        }
        else
        {
            printf("(\\ ");
            kval_print(kv->formals);
            putchar(' ');
            kval_print(kv->body);
            putchar(')');
        }
    }
}

void kval_print_str(kval *v)
{
    char *escaped = malloc(strlen(v->str) + 1);
    strcpy(escaped, v->str);

    escaped = mpcf_escape(escaped);

    printf("\"%s\"", escaped);

    free(escaped);
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

    case KVAL_STR:
        free(kv->str);
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

    case KVAL_FUN:
        if (!kv->fun)
        {
            kenv_del(kv->fenv);
            kval_del(kv->formals);
            kval_del(kv->body);
        }
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

    if (strstr(ast->tag, "string"))
    {
        return kval_read_str(ast);
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
            strcmp(ast->children[i]->tag, "regex") == 0 ||
            strstr(ast->children[i]->tag, "comment"))

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
        if (v->fun)
        {
            x->fun = v->fun;
        }
        else
        {
            x->fun = NULL;
            x->fenv = kenv_copy(v->fenv);
            x->formals = kval_copy(v->formals);
            x->body = kval_copy(v->body);
        }

        break;

    case KVAL_STR:
        x->str = malloc(strlen(v->str) + 1);
        strcpy(x->str, v->str);
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

// Evaluate a kval function
kval *kval_call(kenv *e, kval *f, kval *a)
{

    if (f->fun)
    {
        return f->fun(e, a);
    }

    int given = a->count;
    int total = f->formals->count;

    while (a->count)
    {

        /* If we've run out of formal arguments to bind... */
        if (f->formals->count == 0)
        {
            kval_del(a);
            return kval_err(
                "Function passed too many arguments. "
                "Got %i, Expected %i.",
                given, total);
        }

        /* Pop the first symbol from the formals */
        kval *sym = kval_pop(f->formals, 0);

        /* Special case to deal with '&' */
        if (strcmp(sym->sym, "&") == 0)
        {

            /* Ensure '&' is followed by another symbol */
            if (f->formals->count != 1)
            {
                kval_del(a);
                return kval_err("Function format invalid. "
                                "Symbol '&' not followed by single symbol.");
            }

            /* Next formal should be bound to remaining arguments */
            kval *nsym = kval_pop(f->formals, 0);
            kenv_put(f->fenv, nsym, builtin_list(e, a));

            kval_del(sym);
            kval_del(nsym);
            break;
        }

        /* Pop the next argument from the list */
        kval *val = kval_pop(a, 0);

        /* Bind a copy into the function's environment */
        kenv_put(f->fenv, sym, val);

        /* Delete symbol and value */
        kval_del(sym);
        kval_del(val);
    }

    kval_del(a);

    /* If '&' remains in formal list bind to empty list */
    if (f->formals->count > 0 &&
        strcmp(f->formals->cells[0]->sym, "&") == 0)
    {

        /* Check to ensure that & is not passed invalidly. */
        if (f->formals->count != 2)
        {
            return kval_err("Function format invalid. "
                            "Symbol '&' not followed by single symbol.");
        }

        /* Pop and delete '&' symbol */
        kval_del(kval_pop(f->formals, 0));

        /* Pop next symbol and create empty list */
        kval *sym = kval_pop(f->formals, 0);
        kval *val = kval_qexpr();

        /* Bind to environment and delete */
        kenv_put(f->fenv, sym, val);
        kval_del(sym);
        kval_del(val);
    }

    // If we have all the formals we need to evaluate, then evaluate
    if (f->formals->count == 0)
    {

        f->fenv->parent = e;

        return builtin_eval(f->fenv,
                            kval_add(kval_sexpr(), kval_copy(f->body)));
    }

    // Otherwise, return partially completed function
    return kval_copy(f);
}

int kval_eq(kval *x, kval *y)
{

    /* Different Types are always unequal */
    if (x->type != y->type)
    {
        return 0;
    }

    /* Compare Based upon type */
    switch (x->type)
    {
    /* Compare Number Value */
    case KVAL_NUM:
        return (x->num == y->num);

    /* Compare String Values */
    case KVAL_ERR:
        return (strcmp(x->err, y->err) == 0);

    case KVAL_SYM:
        return (strcmp(x->sym, y->sym) == 0);

    /* If fun compare, otherwise compare formals and body */
    case KVAL_FUN:
        if (x->fun || y->fun)
        {
            return x->fun == y->fun;
        }
        else
        {
            return kval_eq(x->formals, y->formals) && kval_eq(x->body, y->body);
        }

    case KVAL_STR:
        return (strcmp(x->str, y->str) == 0);

    /* If list compare every individual element */
    case KVAL_QEXPR:
    case KVAL_SEXPR:
        if (x->count != y->count)
        {
            return 0;
        }
        for (int i = 0; i < x->count; i++)
        {
            /* If any element not equal then whole list not equal */
            if (!kval_eq(x->cells[i], y->cells[i]))
            {
                return 0;
            }
        }
        /* Otherwise lists must be equal */
        return 1;
        break;
    }
    return 0;
}

kval *kval_str(char *s)
{
    kval *v = malloc(sizeof(kval));
    v->type = KVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
    return v;
}

kval *kval_read_str(mpc_ast_t *t)
{
    t->contents[strlen(t->contents) - 1] = '\0';

    char *unescaped = malloc(strlen(t->contents + 1) + 1);
    strcpy(unescaped, t->contents + 1);
    unescaped = mpcf_unescape(unescaped);

    kval *str = kval_str(unescaped);

    free(unescaped);

    return str;
}
