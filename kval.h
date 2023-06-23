#ifndef kval_h
#define kval_h

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

enum
{
    KVAL_NUM,
    KVAL_ERR,
    KVAL_SYM,
    KVAL_SEXPR,
    KVAL_QEXPR
};

kval *kval_num(long num);
kval *kval_err(char *err);
kval *kval_sym(char *s);
kval *kval_sexpr(void);
kval *kval_qexpr(void);

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
kval *kval_join(kval *x, kval *y);

#endif