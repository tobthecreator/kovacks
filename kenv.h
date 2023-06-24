#ifndef kenv_h
#define kenv_h

#include <stdlib.h>
#include "kval.h"
#include "types.h"

kenv *kenv_init(void);
kval *kenv_get(kenv *e, kval *k);
void kenv_put(kenv *e, kval *k, kval *v);
void kenv_add_builtin(kenv *e, char *name, kbuiltin func);
void kenv_add_builtins(kenv *e);
void kenv_del(kenv *e);
void kenv_def(kenv *e, kval *k, kval *v);
kenv *kenv_copy(kenv *e);

#endif
