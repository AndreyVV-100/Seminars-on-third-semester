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
#define   BUF_SIZE    1024
const int WRITE_SEM = 0;
const int  READ_SEM = 1;
int SEM_IND = -1;

//                              WRITE      READ
struct sembuf CHANGE_SEM[2] = {{0, 1, 0}, {1, 0, 0}};

void SendFile (int fd, char* mem);
void ChangeMode (int write, int read);

// sender
int main (int argc, char** argv)
{
    if (argc != 2)
        return 0;

    int fd = open (argv[1], O_RDONLY);
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

    ChangeMode (1, 0);
    SendFile (fd, mem);
    
    close (fd);
    shmctl (mem_ind, IPC_RMID, NULL);
    semctl (SEM_IND, 1, IPC_RMID);
    return 0;
}

void SendFile (int fd, char* mem)
{
    ssize_t write_size = 0, 
            *buf_info  = (ssize_t*) mem;
    char buf[BUF_SIZE] = "";

    while ((write_size = read (fd, buf, BUF_SIZE)) > 0)
    {
        ChangeMode (-1, 0);
        *buf_info = write_size;
        memcpy (mem + sizeof (ssize_t), buf, write_size);
        ChangeMode (0, 1);
    }

    ChangeMode (-1, 0);
    *buf_info = 0;
    ChangeMode (0, 1);
    return;
}

void ChangeMode (int write, int read)
{
    CHANGE_SEM[WRITE_SEM].sem_op = write;
    CHANGE_SEM[READ_SEM].sem_op  = read;
    semop (SEM_IND, CHANGE_SEM, 2);
    return;
}
