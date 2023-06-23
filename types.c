#include "types.h"

char *ktype_name(int t)
{
    switch (t)
    {
    case KVAL_FUN:
        return "Function";
    case KVAL_NUM:
        return "Number";
    case KVAL_ERR:
        return "Error";
    case KVAL_SYM:
        return "Symbol";
    case KVAL_SEXPR:
        return "S-Expression";
    case KVAL_QEXPR:
        return "Q-Expression";
    default:
        return "Unknown";
    }
}