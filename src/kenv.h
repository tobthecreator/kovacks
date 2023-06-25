#ifndef kenv_h
#define kenv_h

#include <stdlib.h>
#include "kval.h"
#include "types.h"

// Constructor
kenv *kenv_init(void);
void kenv_del(kenv *e);

kval *kenv_get(kenv *e, kval *k);
void kenv_put(kenv *e, kval *k, kval *v);

void kenv_def(kenv *e, kval *k, kval *v);

void kenv_add_builtin(kenv *e, char *name, kbuiltin func);
void kenv_add_builtins(kenv *e);

kenv *kenv_copy(kenv *e);

#endif
