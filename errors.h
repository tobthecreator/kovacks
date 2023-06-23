#ifndef errors_h
#define errors_h

#define KERR_DIV_ZERO "Division by zero"
#define KERR_BAD_OP "Invalid operation"
#define KERR_BAD_NUM "Invalid number"
#define KERR_UNSUPPORTED_TYPE "Unsupported type"
#define KERR_BAD_SEXPR "Invalid S-expression"
#define KERR_UNKNOWN "Unknown"

#define K_ASSERT(args, cond, err) \
    if (!(cond))                 \
    {                            \
        kval_del(args);          \
        return kval_err(err);    \
    }

#endif