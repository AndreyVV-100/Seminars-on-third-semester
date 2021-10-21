#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

const int KEY_CONST = 300;
const int BUF_SIZE  = 1024;
const int WRITE_SEM = 0;
const int  READ_SEM = 1;

int SEM_IND = -1;

//                              WRITE      READ
struct sembuf CHANGE_SEM[2] = {{0, 0, 0}, {1, 0, 0}};

void TakeFile (int fd, char* mem);
void ChangeMode (int write, int read);

// sender
int main (int argc, char** argv)
{
    if (argc != 2)
        return 0;

    int fd = open (argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
        return 0;

    key_t key = ftok ("sender.c", KEY_CONST);
    SEM_IND = semget (key, 2, IPC_CREAT | 0666); 
    if (SEM_IND < 0)
        return 0;

    int mem_ind = shmget (key, BUF_SIZE + sizeof (size_t), IPC_CREAT | 0666);
    if (mem_ind < 0)
        return 0;

    char* mem = (char*) shmat (mem_ind, NULL, 0);
    if (mem == (void*) (-1))
        return 0;

    // ToDo: close all
    TakeFile (fd, mem);
    
    close (fd);
    shmctl (mem_ind, IPC_RMID, NULL);
    semctl (SEM_IND, 1, IPC_RMID);
    return 0;
}

void TakeFile (int fd, char* mem)
{
    ssize_t* buf_info  = (ssize_t*) mem;

    while (1)
    {
        ChangeMode (0, -1);

        if (*buf_info == 0)
        {
            ChangeMode (1, 0);
            break;
        }
        
        write (fd, mem + 8, *buf_info);
        ChangeMode (1, 0);
    }

    return;
}

void ChangeMode (int write, int read)
{
    CHANGE_SEM[WRITE_SEM].sem_op = write;
    CHANGE_SEM[READ_SEM].sem_op  = read;
    semop (SEM_IND, CHANGE_SEM, 2);
    return;
}
