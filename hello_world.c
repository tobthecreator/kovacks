#include <stdio.h>
#include <string.h>

int main_original(int argc, char **argv)
{
    puts("hello, world!");

    return 0;
}

int main(int argc, char **argv)
{
    char hw[] = "hello tyler";
    char *hwp = &hw[0];

    // iterate via the array structure
    for (int i = 0; i <= strlen(hw); i++)
    {
        printf("%c", hw[i]);
    }

    printf("\n");

    // just testing some old memories
    // iterate through the string via memory address
    for (int i = 0; i <= strlen(hw); i++)
    {
        printf("%c", *hwp);

        hwp += 1;
    }

    // print that whole dang thang
    printf("\n");
    puts(hw);

    return 0;
}
