#include <stdio.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>

// read from some FIFO
int main (int argc, char** argv)
{
    if (argc == 1)
        return 0;

    struct pollfd* pfd = (struct pollfd*) calloc (argc - 1, sizeof (*pfd));

    for (int i_file = 0; i_file < argc - 1; i_file++)
    {
        pfd[i_file].fd = open (argv[i_file + 1], O_RDONLY | O_NONBLOCK);
        printf ("AAAAAA\n");

        if (pfd[i_file].fd == -1)
            return 0;

        pfd[i_file].events = POLLIN;
    }

    while (1)
    {
        poll (pfd, argc - 1, -1);

        for (int i_file = 0; i_file < argc - 1; i_file++)
        {
            if (pfd[i_file].revents & POLLIN)
            {
                char buf = '\0';
                while (read (pfd[i_file].fd, &buf, 1) > 0)
                    printf ("%c", buf);
                pfd[i_file].revents = 0;
            }
        }
    }

    return 0;
}
