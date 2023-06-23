#include <stdbool.h>
#include "errors.h"
#include "kval.h"
#include "builtin.h"

kval *builtin(kval *a, char *func)
{
    if (strcmp("list", func) == 0)
    {
        return builtin_list(a);
    }
    if (strcmp("head", func) == 0)
    {
        return builtin_head(a);
    }
    if (strcmp("tail", func) == 0)
    {
        return builtin_tail(a);
    }
    if (strcmp("join", func) == 0)
    {
        return builtin_join(a);
    }
    if (strcmp("eval", func) == 0)
    {
        return builtin_eval(a);
    }
    if (strstr("+-/*", func))
    {
        return builtin_op(a, func);
    }
    kval_del(a);
    return kval_err("Unknown Function!");
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

kval *builtin_head(kval *a)
{
    LASSERT(a, a->count == 1,
            "Function 'head' passed too many arguments!");

    LASSERT(a, a->cells[0]->type == KVAL_QEXPR,
            "Function 'head' passed incorrect type!");

    LASSERT(a, a->cells[0]->count != 0,
            "Function 'head' passed {}!");

    kval *v = kval_take(a, 0);
    while (v->count > 1)
    {
        kval_del(kval_pop(v, 1));
    }
    return v;
}

kval *builtin_tail(kval *a)
{
    LASSERT(a, a->count == 1,
            "Function 'tail' passed too many arguments!");

    LASSERT(a, a->cells[0]->type == KVAL_QEXPR,
            "Function 'tail' passed incorrect type!");

    LASSERT(a, a->cells[0]->count != 0,
            "Function 'tail' passed {}!");

    kval *v = kval_take(a, 0);
    kval_del(kval_pop(v, 0));
    return v;
}

kval *builtin_list(kval *a)
{
    a->type = KVAL_QEXPR;
    return a;
}

kval *builtin_eval(kval *a)
{
    LASSERT(a, a->count == 1,
            "Function 'eval' passed too many arguments!");

    LASSERT(a, a->cells[0]->type == KVAL_QEXPR,
            "Function 'eval' passed incorrect type!");

    kval *x = kval_take(a, 0);
    x->type = KVAL_SEXPR;
    return kval_eval(x);
}

kval *builtin_join(kval *a)
{

    for (int i = 0; i < a->count; i++)
    {
        LASSERT(a, a->cells[i]->type == KVAL_QEXPR,
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
