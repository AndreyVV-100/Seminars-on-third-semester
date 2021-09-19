#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>

int MyCp (const char* file_in, const char* file_out);
off_t GetFileSize (int fd);

#define BUF_SIZE 1024

// my realization of cp
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf (stderr, "Error: Wrong number of parameters.\n");
        return 0;
    }

    int error_stat = MyCp (argv[1], argv[2]);

    if (error_stat)
        perror ("MyCp: ");
    
    return 0;
}

int MyCp (const char* file_in, const char* file_out)
{
    assert (file_in);
    assert (file_out);

    int fd_in  = open (file_in, O_RDONLY);
    int fd_out = open (file_out, O_WRONLY | O_CREAT, 0666);
    char buf[BUF_SIZE] = "";

    #define return_err {                    \
                           close (fd_in);   \
                           close (fd_out);  \
                           return 1;        \
                       }

    if (fd_in == -1 || fd_out == -1)
        return_err;

    int read_num = -1;
    while ((read_num = read (fd_in, buf, BUF_SIZE)) > 0)
    {
        if (write (fd_out, buf, read_num) != read_num)
            return_err;
    }

    if (read_num == -1)
        return_err;

    close (fd_in); 
    close (fd_out);
    return 0;   
}
