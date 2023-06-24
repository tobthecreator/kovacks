#ifndef errors_h
#define errors_h

// #include "types.h"

#define KERR_DIV_ZERO "Division by zero"
#define KERR_BAD_OP "Invalid operation"
#define KERR_BAD_NUM "Invalid number"
#define KERR_UNSUPPORTED_TYPE "Unsupported type"
#define KERR_BAD_SEXPR "Invalid S-expression"
#define KERR_UNKNOWN "Unknown"

#define K_ASSERT(args, cond, fmt, ...)            \
    if (!(cond))                                  \
    {                                             \
        kval *err = kval_err(fmt, ##__VA_ARGS__); \
        kval_del(args);                           \
        return err;                               \
    }

#endif

#define K_ASSERT_TYPE(func, args, index, expect)                     \
    K_ASSERT(args, args->cells[index]->type == expect,               \
             "Function '%s' passed incorrect type for argument %i. " \
             "Got %s, Expected %s.",                                 \
             func, index, ktype_name(args->cells[index]->type), ktype_name(expect))

#define K_ASSERT_NUM(func, args, num)                               \
    K_ASSERT(args, args->count == num,                              \
             "Function '%s' passed incorrect number of arguments. " \
             "Got %i, Expected %i.",                                \
             func, args->count, num)

#define K_ASSERT_NOT_EMPTY(func, args, index)      \
    K_ASSERT(args, args->cells[index]->count != 0, \
             "Function '%s' passed {} for argument %i.", func, index);
