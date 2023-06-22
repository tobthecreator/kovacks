#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

int main()
{
    puts("Kovacs Version 0.0.0.0.1");
    puts("Press Crtl+C to Exit\n");

    while (1)
    {
        // Prompt
        char *input = readline("kovacs> ");

        // Remember Prompts
        add_history(input);

        // Reply
        printf("no You're a %s\n", input);

        free(input);
    }

    return 0;
}