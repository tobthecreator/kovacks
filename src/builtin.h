#ifndef builtin_h
#define builtin_h

#include <string.h>
#include "kval.h"
#include "types.h"

kval *builtin(kenv *e, kval *a, char *func);
kval *builtin_op(kenv *e, kval *kv, char *op);

// #################
//  List Functions #
// #################
kval *builtin_head(kenv *e, kval *a);
kval *builtin_tail(kenv *e, kval *a);
kval *builtin_list(kenv *e, kval *a);
kval *builtin_eval(kenv *e, kval *a);
kval *builtin_join(kenv *e, kval *a);

// #################
//  Math Functions #
// #################
kval *builtin_add(kenv *e, kval *a);
kval *builtin_sub(kenv *e, kval *a);
kval *builtin_mul(kenv *e, kval *a);
kval *builtin_div(kenv *e, kval *a);

// ########################
//  Function... Functions #
// ########################
kval *builtin_def(kenv *e, kval *a);
kval *builtin_put(kenv *e, kval *a);
kval *builtin_var(kenv *e, kval *a, char *func);

// #################
//  Conditionals   #
// #################
kval *builtin_gt(kenv *e, kval *a);
kval *builtin_lt(kenv *e, kval *a);
kval *builtin_ge(kenv *e, kval *a);
kval *builtin_le(kenv *e, kval *a);
kval *builtin_ord(kenv *e, kval *a, char *op);
kval *builtin_cmp(kenv *e, kval *a, char *op);
kval *builtin_eq(kenv *e, kval *a);
kval *builtin_ne(kenv *e, kval *a);
kval *builtin_if(kenv *e, kval *a);

// #################
//  Strings        #
// #################
kval *builtin_error(kenv *e, kval *a);
kval *builtin_print(kenv *e, kval *a);

// #################
// Files           #
// #################
kval *builtin_load(kenv *e, kval *a);

#endif