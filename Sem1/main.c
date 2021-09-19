#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

void PrintArg (char* arg);
void SkipTabs (char** arg);

const int NO_NUMS = 228;

// simple bizzbuzz
int main (int argc, char** argv)
{
    for (int i_arg = 1; i_arg < argc; i_arg++)
        PrintArg (argv[i_arg]);

    printf ("\n");
    return 0;
}

void PrintArg (char* arg)
{
    assert (arg);

    char* save_arg = arg;
    int last_num = NO_NUMS;
    int sum_in_3 = 0;

    SkipTabs (&arg);
    if (*arg == '-')
        arg++;

    while (isdigit (*arg))
    {
        last_num = *arg - '0';
        sum_in_3 = (sum_in_3 + last_num) % 3;
        arg++;
    }

    SkipTabs (&arg);
    if (*arg || last_num == NO_NUMS || (sum_in_3 != 0 && last_num % 5 != 0))
    {
        printf ("%s ", save_arg);
        return;
    }

    if (sum_in_3 == 0)
        printf ("bizz");

    if (last_num == 0 || last_num == 5)
        printf ("buzz");

    printf (" ");
    return; 
}

void SkipTabs (char** arg)
{
    assert (arg);
    assert (*arg);

    while (**arg == ' ' || **arg == '\t' || **arg == '\n' || **arg == '\r')
        arg++;

    return;
}
