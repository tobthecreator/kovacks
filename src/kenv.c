#include "kenv.h"
#include "kval.h"
#include "builtin.h"
#include "errors.h"

// Constructor
kenv *kenv_init(void)
{
    kenv *e = malloc(sizeof(kenv));

    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    e->parent = NULL;

    return e;
}

// Deconstructor
void kenv_del(kenv *e)
{
    for (int i = 0; i < e->count; i++)
    {
        free(e->syms[i]);
        kval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

// ############
//  Helpers   #
// ############
kval *kenv_get(kenv *e, kval *k)
{
    // Look through known variables
    for (int i = 0; i < e->count; i++)
    {
        // Check if the stored string matches the symbol string
        if (strcmp(e->syms[i], k->sym) == 0)
        {
            return kval_copy(e->vals[i]);
        }
    }

    if (e->parent)
    {
        return kenv_get(e->parent, k);
    }

    // TODO - add a real error code here
    return kval_err("Unbound symbol: '%s'", k->sym);
}

// Put definition in the local environment
void kenv_put(kenv *e, kval *k, kval *v)
{

    // Look through known variables
    for (int i = 0; i < e->count; i++)
    {

        // If an existing variable is found, overwrite it
        if (strcmp(e->syms[i], k->sym) == 0)
        {
            kval_del(e->vals[i]);
            e->vals[i] = kval_copy(v);
            return;
        }
    }

    // If not found, add it
    e->count++;
    e->vals = realloc(e->vals, sizeof(kval *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);

    e->vals[e->count - 1] = kval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}

// Put definition in the global environment
void kenv_def(kenv *e, kval *k, kval *v)
{
    while (e->parent)
    {
        e = e->parent;
    }

    kenv_put(e, k, v);
}

void kenv_add_builtin(kenv *e, char *name, kbuiltin func)
{
    kval *k = kval_sym(name);
    kval *v = kval_fun(func);
    kenv_put(e, k, v);
    kval_del(k);
    kval_del(v);
}

void kenv_add_builtins(kenv *e)
{
    /* List Functions */
    kenv_add_builtin(e, "list", builtin_list);
    kenv_add_builtin(e, "head", builtin_head);
    kenv_add_builtin(e, "tail", builtin_tail);
    kenv_add_builtin(e, "eval", builtin_eval);
    kenv_add_builtin(e, "join", builtin_join);

    /* Mathematical Functions */
    kenv_add_builtin(e, "+", builtin_add);
    kenv_add_builtin(e, "-", builtin_sub);
    kenv_add_builtin(e, "*", builtin_mul);
    kenv_add_builtin(e, "/", builtin_div);

    /* Functions... Functions */
    kenv_add_builtin(e, "def", builtin_def);
    kenv_add_builtin(e, "=", builtin_put);
    kenv_add_builtin(e, "\\", builtin_lambda);

    /* Conditionals */
    kenv_add_builtin(e, "if", builtin_if);
    kenv_add_builtin(e, "==", builtin_eq);
    kenv_add_builtin(e, "!=", builtin_ne);
    kenv_add_builtin(e, ">", builtin_gt);
    kenv_add_builtin(e, "<", builtin_lt);
    kenv_add_builtin(e, ">=", builtin_ge);
    kenv_add_builtin(e, "<=", builtin_le);

    /* String Functions */
    kenv_add_builtin(e, "load", builtin_load);
    kenv_add_builtin(e, "error", builtin_error);
    kenv_add_builtin(e, "print", builtin_print);
}

kenv *kenv_copy(kenv *e)
{
    kenv *n = malloc(sizeof(kenv));
    n->parent = e->parent;
    n->count = e->count;
    n->syms = malloc(sizeof(char *) * n->count);
    n->vals = malloc(sizeof(kval *) * n->count);

    for (int i = 0; i < e->count; i++)
    {
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = kval_copy(e->vals[i]);
    }
    return n;
}