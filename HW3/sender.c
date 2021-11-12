#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "config.h"

int NOT_READY = 0,
    PID = 0,
    SEM_IND = 0;
struct sembuf CHANGE_SEM = {0, -1, 0};


typedef struct
{
    size_t filesize;
    int fd;
} File;

int GetFile (const char* filename, File* file);
void SendFile (File file);

// sending file by signals
// argv[1] - file in, argv[2] - receiver pid
int main (int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf (stderr, "Bad num of arguments.\n");
        return 0;
    }

    File file = {};
    if (GetFile (argv[1], &file))
        return 0;

    PID = atoi (argv[2]);
    if (PID <= 0 || kill (PID, 0) == -1)
    {
        fprintf (stderr, "Bad receiver pid...\n");
        close (file.fd);
        return 0;
    }

    key_t key = ftok ("sender.c", KEY_CONST);
    SEM_IND = semget (key, 2, IPC_CREAT | 0666); 
    if (SEM_IND < 0)
    {
        perror ("Something went wrong with sem");
        close (file.fd);
        return 0;
    }

    SendFile (file);

    close (file.fd);
    semctl (SEM_IND, 1, IPC_RMID);
    return 0;
}

int GetFile (const char* filename, File* file)
{
    *file = (File) {0, -1};
    struct stat file_stat = {};
    
    if (stat (filename, &file_stat))
    {
        perror (filename);
        return -1;
    }
    file->filesize = file_stat.st_size;

    if ((file->fd = open (filename, O_RDONLY)) == -1)
    {
        perror (filename);
        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------------------------

#define SEND_SIG(sig_num) if (kill (PID, sig_num))               \
                          {                                      \
                              fprintf (stderr, "%d\n", sig_num); \
                              perror ("Send error" #sig_num);    \
                              return;                            \
                          }

// first - low, second, high byte
void SendFile (File file)
{
    semop (SEM_IND, &CHANGE_SEM, 1);
    SEND_SIG (SIGUSR1); // start
    semop (SEM_IND, &CHANGE_SEM, 1);

    for (size_t i_size = 0; i_size < file.filesize; i_size++)
    {
        char buf = 0;
        if (read (file.fd, &buf, 1) != 1)
        {
            perror ("Read error");
            return;
        }

        #ifdef DEBUG
            printf ("Config:\n"
                    "  i_size = %zu\n"
                    "  byte   = %d\n" 
                    "  low    = %d\n" 
                    "  high   = %d\n\n", i_size,
                                         (int)   buf,
                                         (int)  (buf & LOW_HALF_BYTE_MASK) + ALIGN,
                                         (int) ((buf & HIGH_HALF_BYTE_MASK) >> HALF_BYTE_SIZE) + ALIGN);
        #endif

        SEND_SIG ((buf & LOW_HALF_BYTE_MASK) + ALIGN);
        semop (SEM_IND, &CHANGE_SEM, 1);
        SEND_SIG (((buf & HIGH_HALF_BYTE_MASK) >> HALF_BYTE_SIZE) + ALIGN);
        semop (SEM_IND, &CHANGE_SEM, 1);
    }

    SEND_SIG (SIGUSR2); // stop
    return;
}
#undef SEND_SIG
