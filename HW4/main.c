#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include "stack.h"

// stack on shared memory
int main()
{
    key_t key = ftok ("stack.c", 228);

    struct stack_t* stack = attach_stack (key, 1024);
    mark_destruct (stack);
    int original = 0;

    for (int i_popa = 0; i_popa < 10; i_popa++)
        original += !fork ();

    push (stack, getpid());
    size_t val = 0;
    pop (stack, &val);

    if (!original)
    {
        sleep (5);
        printf ("%d\n", get_size (stack));
    }

    detach_stack (stack);
    return 0;
}