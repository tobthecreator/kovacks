#!/bin/bash

mkdir -p ./dist

cc -std=c99 -Wall $(find ./src -maxdepth 1 -name '*.c') -ledit -lm -o ./dist/kovacs.out

cp ./src/libs/stdlib.k ./dist