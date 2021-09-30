#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main()
{
    const int PROCESS_NUM = 1024;
    srand (time (NULL));

    for (int i_pr = 0; i_pr < PROCESS_NUM; i_pr++)
    {
        int ret = rand();
        if (!fork()) // ToDo: think about it
            return ret;
    }

    int fd = -1;
    char buf[256] = "";
    if ((fd = open ("out.txt", O_WRONLY | O_CREAT, 0666)) == -1)
        return 0;

    for (int i_pr = 0; i_pr < PROCESS_NUM; i_pr++)
    {
        int wstatus = 0;
        int child = wait (&wstatus);

        if (WIFEXITED (wstatus))
        {
            int size = sprintf (buf, "%d: %d\n", child, WEXITSTATUS (wstatus));
            if (write (fd, buf, size) == -1)
                return 0;
        }
    }

    close (fd);
    return 0;
}
