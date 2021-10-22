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
    int read_num = -1;

    key_t key = ftok ("sender.c", KEY_CONST);
    int msg_ind = msgget (key, IPC_CREAT | 0666);

    msgbuf buf = {1, ""};
    while ((read_num = read (STDIN_FILENO, buf.buf, BUF_SIZE - 1)) > 0)
    {
        buf.buf[read_num] = '\0';
        msgsnd (msg_ind, &buf, BUF_SIZE, 0);
    }

    buf.mtype = 2;
    msgsnd (msg_ind, &buf, BUF_SIZE, 0);
    msgctl (msg_ind, IPC_RMID, NULL);
    return 0;
}
