#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"

int FD = -1,
    PID = 0,
    NOT_SENDED = 1;
struct sembuf CHANGE_SEM = {0, -1, 0};

int GetFile (const char* filename);
void ConfigSending (int sig, siginfo_t* info, __attribute__((unused)) void* zachem_eto_pole);
void SendFile();

#define SET_ACT(sig_num) if (sigaction (sig_num, &sig_act, NULL)) \
                         {                                        \
                             perror ("Something went wrong");     \
                             close (FD);                          \
                             return 0;                            \
                         }

#define SEND_SIG(sig_num) if (kill (PID, sig_num))               \
                          {                                      \
                              fprintf (stderr, "%d\n", sig_num); \
                              perror ("Send error " #sig_num);   \
                          }

// WARNING: Never use this code
// sending file by signals
// argv[1] - file in, argv[2] - receiver pid
int main (int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf (stderr, "Bad num of arguments.\n");
        return 0;
    }

    if ((FD = open (argv[1], O_RDONLY)) == -1)
    {
        perror (argv[1]);
        return -1;
    }

    PID = atoi (argv[2]);
    if (PID <= 0 || kill (PID, 0) == -1)
    {
        fprintf (stderr, "Bad receiver pid...\n");
        close (FD);
        return 0;
    }

    struct sigaction sig_act = {.sa_sigaction = SendFile, .sa_flags = SA_SIGINFO};
    sig_act.sa_sigaction = ConfigSending;
    SET_ACT (SIGUSR1);
    SET_ACT (SIGUSR2);
    SET_ACT (SIGINT);

    struct timespec time_begin = {},
                    time_end   = {};
    clock_gettime (CLOCK_REALTIME, &time_begin);
    SEND_SIG (SIGUSR1);
    while (NOT_SENDED) { ; } // Warning: dangerous
    clock_gettime (CLOCK_REALTIME, &time_end);

    struct stat info = {};
    stat (argv[1], &info);
    printf ("Send speed = %lf Bps.\n", (double) info.st_size /
                (time_end.tv_sec - time_begin.tv_sec +
                1e-9 * (time_end.tv_nsec - time_begin.tv_nsec)));

    close (FD);
    return 0;
}

#undef SET_ACT

//------------------------------------------------------------------------------------------------

void ConfigSending (int sig, siginfo_t* info, __attribute__((unused)) void* zachem_eto_pole)
{
    #ifdef DEBUG
        printf ("Config:\n"
                "  pid = %d\n"
                "  signal = %d\n\n", info->si_pid, sig);
    #endif

    if (info->si_pid == PID && sig == SIGUSR1 && NOT_SENDED)
    {
        SendFile();
    }
    
    else if (PID == info->si_pid && sig == SIGUSR2)
        NOT_SENDED = 0;

    else if (sig == SIGINT)
    {
        NOT_SENDED = 0;
        if (PID)
            SEND_SIG (SIGUSR2);
    }

    return;
}

// first - low, second, high byte
void SendFile ()
{
    static char buf = 0;
    static int half_part = 0;

    if (!half_part)
    {
        int res = read (FD, &buf, 1);

        if (res == 0)
        {
            SEND_SIG (SIGUSR2); // stop
            NOT_SENDED = 0;
            return;
        }
        else if (res != 1)
        {
            perror ("Read error");
            SEND_SIG (SIGUSR2);
            return;
        }
    }

    #ifdef DEBUG
        printf ("Config:\n"
                "  byte   = %d\n" 
                "  low    = %d\n" 
                "  high   = %d\n\n",    (int)   buf,
                                        (int)  (buf & LOW_HALF_BYTE_MASK) + ALIGN,
                                        (int) ((buf & HIGH_HALF_BYTE_MASK) >> HALF_BYTE_SIZE) + ALIGN);
    #endif

    if (!half_part)
    {
        half_part = 1;
        SEND_SIG ((buf & LOW_HALF_BYTE_MASK) + ALIGN);
    }
    else
    {
        half_part = 0;
        SEND_SIG (((buf & HIGH_HALF_BYTE_MASK) >> HALF_BYTE_SIZE) + ALIGN);
    }
    return;
}
#undef SEND_SIG
