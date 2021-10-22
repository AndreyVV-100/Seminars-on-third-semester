#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

const int KEY_CONST = 300;
#define BUF_SIZE 1024

typedef struct
{
    long mtype;
    char buf[BUF_SIZE];
} msgbuf;

int main()
{
    key_t key = ftok ("sender.c", KEY_CONST);
    int msg_ind = msgget (key, IPC_CREAT | 0666);

    msgbuf buf = {};
    while (1)
    {
        msgrcv (msg_ind, &buf, BUF_SIZE, 0, 0);
        if (buf.mtype == 2)
            break;
        printf ("%s", buf.buf);
    }
    
    msgctl (msg_ind, IPC_RMID, NULL);
    return 0;
}
