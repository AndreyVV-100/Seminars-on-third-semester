#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include "stack.h"

// stack on shared memory

    #include "stack.h"
    #include <stdlib.h>

int main()
{
    int error = 0;
    key_t key = ftok ("stack.c", 228);
    size_t stack_size = 30;

    pid_t pid = 0;

    for (int i = 0; i < 4; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork() error");
            exit(1);
        }
    }

    struct stack_t* stack = attach_stack(key, stack_size);
    if (stack == NULL)
    {
        perror ("");
        return 0;
    }

    size_t value = 21;

    push(stack, value);
    //pop(stack, (void **) &value);

    mark_destruct(stack);

    detach_stack(stack);
    // perror ("");

    return 0;
}

