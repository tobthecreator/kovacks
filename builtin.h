#ifndef builtin_h
#define builtin_h

#include <string.h>
#include "kval.h"

kval *builtin_op(kval *kv, char *op);
kval *builtin_head(kval *a);
kval *builtin_tail(kval *a);
kval *builtin_list(kval *a);
kval *builtin_eval(kval *a);
kval *builtin_join(kval *a);
kval *builtin(kval *a, char *func);

#endif