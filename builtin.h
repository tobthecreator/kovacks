#ifndef builtin_h
#define builtin_h

#include <string.h>
#include "kval.h"
#include "types.h"

kval *builtin_op(kenv *e, kval *kv, char *op);
kval *builtin_head(kenv *e, kval *a);
kval *builtin_tail(kenv *e, kval *a);
kval *builtin_list(kenv *e, kval *a);
kval *builtin_eval(kenv *e, kval *a);
kval *builtin_join(kenv *e, kval *a);
kval *builtin(kenv *e, kval *a, char *func);
kval *builtin_add(kenv *e, kval *a);
kval *builtin_sub(kenv *e, kval *a);
kval *builtin_mul(kenv *e, kval *a);
kval *builtin_div(kenv *e, kval *a);
kval *builtin_def(kenv *e, kval *a);
kval *builtin_put(kenv *e, kval *a);
kval *builtin_var(kenv *e, kval *a, char *func);

#endif