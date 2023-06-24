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

## Running a .k file

You can run as many .k files as you'd like with:

```
./kovacs.out file1.k file2.k file3.k
```

These files will execute in the sequence provided.

# Kovacs Definitions
