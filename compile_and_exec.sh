#!/bin/bash

# # Compile all .c files and generate object files
# for file in *.c; do
#     object_file="${file%.*}.o"
#     cc -std=c99 -Wall -c "$file" -o "$object_file"
# done

# # Link object files and create the final executable
# cc -std=c99 -Wall *.o -ledit -lm -o kovacs.out && ./kovacs.out

# # Clean up object files
# rm -f *.o

cc -std=c99 -Wall $(find . -maxdepth 1 -name '*.c') -ledit -lm -o kovacs.out && ./kovacs.out
