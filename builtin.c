#include <stdbool.h>
#include "errors.h"
#include "kval.h"
#include "builtin.h"
#include "types.h"
#include "kenv.h"

kval *builtin(kenv *e, kval *a, char *func)
{
    if (strcmp("list", func) == 0)
    {
        return builtin_list(e, a);
    }

    if (strcmp("head", func) == 0)
    {
        return builtin_head(e, a);
    }

    if (strcmp("tail", func) == 0)
    {
        return builtin_tail(e, a);
    }

    if (strcmp("join", func) == 0)
    {
        return builtin_join(e, a);
    }

    if (strcmp("eval", func) == 0)
    {
        return builtin_eval(e, a);
    }

    if (strstr("+-/*", func))
    {
        return builtin_op(e, a, func);
    }

    kval_del(a);
    return kval_err("Unknown Function!");
}

kval *builtin_op(kenv *e, kval *kv, char *op)
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

kval *builtin_head(kenv *e, kval *a)
{
    K_ASSERT(a, a->count == 1,
             "Function 'head' passed too many arguments!\nGot %i, Expected %i",
             a->count, 1);

    K_ASSERT(a, a->cells[0]->type == KVAL_QEXPR,
             "Function 'head' passed incorrect type for argument 0!\nGot %s, Expected %s.",
             ktype_name(a->cells[0]->type),
             ktype_name(KVAL_QEXPR));

    K_ASSERT(a, a->cells[0]->count != 0,
             "Function 'head' passed {}!");

    kval *v = kval_take(a, 0);
    while (v->count > 1)
    {
        kval_del(kval_pop(v, 1));
    }
    return v;
}

kval *builtin_tail(kenv *e, kval *a)
{
    K_ASSERT(a, a->count == 1,
             "Function 'tail' passed too many arguments!");

    K_ASSERT(a, a->cells[0]->type == KVAL_QEXPR,
             "Function 'tail' passed incorrect type!");

    K_ASSERT(a, a->cells[0]->count != 0,
             "Function 'tail' passed {}!");

    kval *v = kval_take(a, 0);
    kval_del(kval_pop(v, 0));
    return v;
}

kval *builtin_list(kenv *e, kval *a)
{
    a->type = KVAL_QEXPR;
    return a;
}

kval *builtin_eval(kenv *e, kval *a)
{
    K_ASSERT(a, a->count == 1,
             "Function 'eval' passed too many arguments!");

    K_ASSERT(a, a->cells[0]->type == KVAL_QEXPR,
             "Function 'eval' passed incorrect type!");

    kval *x = kval_take(a, 0);
    x->type = KVAL_SEXPR;

    return kval_eval(e, x);
}

kval *builtin_join(kenv *e, kval *a)
{

    for (int i = 0; i < a->count; i++)
    {
        K_ASSERT(a, a->cells[i]->type == KVAL_QEXPR,
                 "Function 'join' passed incorrect type.");
    }

    kval *x = kval_pop(a, 0);

    while (a->count)
    {
        x = kval_join(x, kval_pop(a, 0));
    }

    kval_del(a);
    return x;
}

kval *builtin_add(kenv *e, kval *a)
{
    return builtin_op(e, a, "+");
}

kval *builtin_sub(kenv *e, kval *a)
{
    return builtin_op(e, a, "-");
}

kval *builtin_mul(kenv *e, kval *a)
{
    return builtin_op(e, a, "*");
}

kval *builtin_div(kenv *e, kval *a)
{
    return builtin_op(e, a, "/");
}

kval *builtin_def(kenv *e, kval *a)
{
    return builtin_var(e, a, "def");
}

kval *builtin_put(kenv *e, kval *a)
{
    return builtin_var(e, a, "=");
}

kval *builtin_var(kenv *e, kval *a, char *func)
{
    K_ASSERT_TYPE(func, a, 0, KVAL_QEXPR);

    kval *syms = a->cells[0];
    for (int i = 0; i < syms->count; i++)
    {
        K_ASSERT(a, (syms->cells[i]->type == KVAL_SYM),
                 "Function '%s' cannot define non-symbol. "
                 "Got %s, Expected %s.",
                 func,
                 ktype_name(syms->cells[i]->type),
                 ktype_name(KVAL_SYM));
    }

    K_ASSERT(a, (syms->count == a->count - 1),
             "Function '%s' passed too many arguments for symbols. "
             "Got %i, Expected %i.",
             func, syms->count, a->count - 1);

    for (int i = 0; i < syms->count; i++)
    {
        /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0)
        {
            kenv_def(e, syms->cells[i], a->cells[i + 1]);
        }

        if (strcmp(func, "=") == 0)
        {
            kenv_put(e, syms->cells[i], a->cells[i + 1]);
        }
    }

    kval_del(a);
    return kval_sexpr();
}
