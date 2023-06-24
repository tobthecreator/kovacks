#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_QUOTES 2

const char *quotes[NUM_QUOTES] = {
    "A man who never loves gives no hostage to fortune.\n-Takeshi Kovacs",
    "The human eye is a wonderful device. With a little effort, it can fail to see even the most glaring of bugs.\n-Takeshi Kovacs\n-Tyler O'Briant",
};

void print_altered_carbon_quote()
{
    // Seed the random number generator with current time
    srand(time(NULL));

    // Generate a random index
    int randomIndex = rand() % NUM_QUOTES;

    // Retrieve the random quote
    const char *randomQuote = quotes[randomIndex];

    // Print the random quote
    printf("\n%s\n\n", randomQuote);
}