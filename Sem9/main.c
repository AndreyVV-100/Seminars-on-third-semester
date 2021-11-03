#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef enum
{
    READ  = 0,
    WRITE = 1
} Mode;

void* CreateMmap (const char* path, size_t size, Mode mode);

// cp argv[1] -> argv[2] by mmap
int main (int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf (stderr, "Bad number of parameters.\n");
        return 0;
    }

    struct stat in_stat = {};
    if (stat (argv[1], &in_stat))
    {
        perror (argv[1]);
        return 0;
    }

    void* file_in = CreateMmap (argv[1], in_stat.st_size, READ);
    if (file_in == MAP_FAILED)
        return 0;

    void* file_out = CreateMmap (argv[2], in_stat.st_size, WRITE);
    if (file_out == MAP_FAILED)
    {
        munmap (file_in, in_stat.st_size);
        return 0;
    }

    memcpy (file_out, file_in, in_stat.st_size);
    munmap (file_in,  in_stat.st_size);
    munmap (file_out, in_stat.st_size);
    return 0;
}

void* CreateMmap (const char* path, size_t size, Mode mode)
{
    assert (path);

    int fd = (mode == READ) ? open (path, O_RDONLY) : 
                              open (path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
    {
        perror (path);
        return MAP_FAILED;
    }

    if (mode == WRITE && ftruncate (fd, size))
    {
        perror ("Resize error");
        close (fd);
        return MAP_FAILED;
    }

    char* file = mmap (NULL, size, 
                      (mode == READ) ? PROT_READ : PROT_WRITE,
                       MAP_SHARED, fd, 0);
    
    if (file == MAP_FAILED)
        perror ("Mmap error");
    close (fd);
    return file;
}
