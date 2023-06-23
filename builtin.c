#include <stdbool.h>
#include "errors.h"
#include "kval.h"
#include "builtin.h"

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
