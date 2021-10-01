#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

// my realization ">" in console
int main (int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf (stderr, "Error: too few parameters.\n");
        return 0;
    }

    int new_stdout = -1;
    if ((new_stdout = open (argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
    {
        perror ("Can't open stdout");
        return 0;
    }

    if (!dup2 (new_stdout, STDOUT_FILENO))
    {
        perror ("");
        return 0;
    }

    close (new_stdout);
    execvp (argv[2], argv + 2);
    return 0;
}
