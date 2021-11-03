#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "config.h"

int FD = -1,
    SI_PID = 0,
    NOT_SENDED = 1,
    SEM_IND = 0;
struct sembuf CHANGE_SEM = {0, 1, 0};

typedef struct
{
    size_t filesize;
    int fd;
} File;

int GetFile (const char* filename, int* fd);

void ConfigSending (int sig, siginfo_t* info, void* zachem_eto_pole);
void Handler      (int sig, siginfo_t* info, void* zachem_eto_pole);

//------------------------------------------------------------------------------------------------

#define SET_ACT(sig_num) if (sigaction (sig_num, &sig_act, NULL)) \
                         {                                        \
                             perror ("Something went wrong");     \
                             close (FD);                          \
                             return 0;                            \
                         }

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf (stderr, "Bad num of arguments.\n");
        return 0;
    }

    if (GetFile (argv[1], &FD))
        return 0;

    struct sigaction sig_act = {.sa_sigaction = Handler, .sa_flags = SA_SIGINFO};
    for (int i_act = ALIGN; i_act <= ALIGN + LOW_HALF_BYTE_MASK; i_act++)
    {
        SET_ACT (i_act);
    }

    sig_act.sa_sigaction = ConfigSending;
    SET_ACT (SIGUSR1);
    SET_ACT (SIGUSR2);

    key_t key = ftok ("sender.c", KEY_CONST);
    SEM_IND = semget (key, 2, IPC_CREAT | 0666); 
    if (SEM_IND < 0)
    {
        perror ("Something went wrong with sem");
        close (FD);
        return 0;
    }

    while (NOT_SENDED) sleep (1);
    close (FD);
    semctl (SEM_IND, 1, IPC_RMID);
    return 0;
}
#undef SET_ACT

//------------------------------------------------------------------------------------------------

int GetFile (const char* filename, int* fd)
{
    if ((*fd = open (filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
    {
        perror (filename);
        return -1;
    }
    return 0;
}

void ConfigSending (int sig, siginfo_t* info, __attribute__((unused)) void* zachem_eto_pole)
{
    #ifdef DEBUG
        printf ("Config:\n"
                "  pid = %d\n"
                "  signal = %d\n\n", info->si_pid, sig);
    #endif

    if (SI_PID == 0 && sig == SIGUSR1)
    {
        SI_PID = info->si_pid;
        semop (SEM_IND, &CHANGE_SEM, 1);
    }

    else if (SI_PID == info->si_pid && sig == SIGUSR2)
        NOT_SENDED = 0;

    return;
}

// first - low, second, high byte
void Handler (int sig, siginfo_t* info, __attribute__((unused)) void* zachem_eto_pole)
{
    static int write_now = 0;
    static char buf = 0;

    #ifdef DEBUG
        printf ("Handler:\n"
                "  pid = %d\n"
                "  signal = %d\n", info->si_pid, sig);
    #endif

    if (!SI_PID)
        return;

    if (SI_PID != info->si_pid)
        return;

    if (!write_now)
    {
        write_now = 1;
        buf = (char) (sig - ALIGN);
    }
    else
    {
        buf += ((char) (sig - ALIGN)) << HALF_BYTE_SIZE;
        if (write (FD, &buf, 1) <= 0)
        {
            perror ("Write error");
            NOT_SENDED = 0;
        }
        write_now = 0;
    }

    semop (SEM_IND, &CHANGE_SEM, 1);
    #ifdef DEBUG
        printf ("  Ready signal sended!\n\n");
    #endif
    return;
}
