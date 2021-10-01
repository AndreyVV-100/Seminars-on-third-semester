#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// creates 1024 processes and write information about it
int main()
{
    const int PROCESS_NUM = 1024;

    int fd = -1, child_pid = 0;
    char buf[256] = "";
    unlink ("out.txt");
    if ((fd = open ("out.txt", O_WRONLY | O_CREAT, 0666)) == -1)
        return 0;

    int num_write = sprintf (buf, "PID = %d, return = %d.\n", getpid(), 0); // 1024 processes
    write (fd, buf, num_write);

    // create 1024 = 2^10 processes
    for (int i_proc = 0; i_proc < 10; i_proc++)
    {
        int wstatus = 0;
        fork();
        child_pid = wait (&wstatus);
        if (child_pid > 0 && WIFEXITED (wstatus))
        {
            num_write = sprintf (buf, "PID = %d, return = %d.\n", child_pid, WEXITSTATUS (wstatus));
            write (fd, buf, num_write);
        }
    }

    close (fd);
    srand (child_pid);
    return rand();
}
