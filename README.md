# kovacs

Creating my own Lisp while rewatching Altered Carbon season 1

## Build Instructions

From the main directory of this project, run

```
./scripts/build.sh
```

That should create a `dist` folder with the REPL's executable and the stdlib. The stdlib is read into the REPL at runtime.

## Running the REPL

To run the reply, build the project, then run:

```
cd dist && ./kovacs.out
```

## Installation

I'm not sure why you would want to install this little test LISP on your machine globally, but if you do, both `kovacs.out` and `stdlib.k` need to be in the same folder together. `kovacs.out` looks in it's folder for `stdlib.k` at runtime and loads the stdlib in before opening up the REPL or executing any files.

I've provided a script that moves the contents of `dist` into `~/bin/kovacs` and adds them to your PATH via a .bashrc file. I haven't fully tested this since I am a zsh user and had to wrangle my own .zshrc to get this to work.

```
./scripts/install.sh
```

## Running a .k file

You can run as many .k files as you'd like with:

```
./kovacs.out file1.k file2.k file3.k
```

These files will execute in the sequence provided.

# Kovacs

The following overview is not finished. I need to find a version of this that matches my taste to get a better feel for the structure. I don't expect anyone to see or read this repo, but I want to leave a good paper trail for myself later.

## Standard Library

You can read the full standard library in `src/libs`. The standard library is loaded at runtime, and also accompanies the executable for the REPL. Puruse it at your leasure.

## Numbers... Well, just Integers

Instead of `1 + 2`, everything is written `+ 1 2`. Or, `<operation> <arg1> ... <argn>`

This is a pet project, so right now only integer numbers are supported. The behavior of those integers during division and multiplication should handle exactly like C.

## Strings

We do have strings though! Strings are double quoted.

You cannot do a whole lot with strings at the moment.

You can print store them as variable: `(def {x} "hello!")`

You can print them as strings: `print x` or `(print x)`

Or as errors: `error x` or `(error x)`

## Lists

Lists are defined by curly brackets, `{ }`.

You can create a list with the `list` operation: `(list 1 2 3 4)`.

You can get the length of a list: `(len (list 1 2 3 4))`

Lists are 0-indexed. To get the nth item, you can use the `nth` function: `(nth (list 1 2 3 4))`

## Variables

You can declare a variable using `def`.

## Comments

Comments are left with `;`. Only single-line comments are supported right now
