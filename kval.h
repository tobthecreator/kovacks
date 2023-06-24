#ifndef kval_h
#define kval_h

#include "mpc.h"
#include "types.h"

kval *kval_num(long num);
kval *kval_err(char *err, ...);
kval *kval_sym(char *s);
kval *kval_sexpr(void);
kval *kval_qexpr(void);
kval *kval_fun(kbuiltin func);

void kval_del(kval *kv);
kval *kval_read(mpc_ast_t *ast);
kval *kval_add(kval *kv, kval *new_cell);
kval *kval_read_num(mpc_ast_t *ast);
void kval_println(kval *kv);
void kval_print(kval *kv);
void kval_expr_print(kval *kv, char open, char close);

kval *kval_eval(kenv *e, kval *kv);
kval *kval_eval_sexpr(kenv *e, kval *kv);
kval *kval_pop(kval *kv, int i);
kval *kval_take(kval *v, int i);
kval *kval_join(kval *x, kval *y);
kval *kval_copy(kval *v);
kval *kval_lambda(kval *formals, kval *body);
kval *kval_call(kenv *e, kval *f, kval *a);
int kval_eq(kval *x, kval *y);
void kval_print_str(kval *v);
kval *kval_read_str(mpc_ast_t *t);
kval *kval_str(char *s);

#endif