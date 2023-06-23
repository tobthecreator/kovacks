#ifndef types_h
#define types_h

struct kval;
typedef struct kval kval;

struct kenv;
typedef struct kenv kenv;

enum
{
    KVAL_NUM,
    KVAL_ERR,
    KVAL_SYM,
    KVAL_SEXPR,
    KVAL_QEXPR,
    KVAL_FUN
};

/*
To get an kval*
we dereference kbuiltin and call it with a kenv* and a kval*."

Therefore kbuiltin must be a function pointer
that takes an kenv* and a kval* and returns a kval*.
*/

typedef kval *(*kbuiltin)(kenv *, kval *);

// Kovacs value
struct kval
{
    int type;
    long num;

    char *err;
    char *sym;
    kbuiltin fun;

    int count;
    struct kval **cells;
};

/*
    We're going to store variables in the dumbest way you can think

    Two lists

    The first list are the variable names

    The second list are the variable values
*/
struct kenv
{
    int count;
    char **syms;
    kval **vals;
};

#endif