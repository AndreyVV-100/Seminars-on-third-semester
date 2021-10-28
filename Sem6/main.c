#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void Usr1Handler (int signal);
void Usr2Handler (int signal);

int main()
{
    struct sigaction usr1 = {
        .__sigaction_handler = &Usr1Handler,
        .sa_flags = 0,
        .sa_mask = 0};
    sigaction (SIGUSR1, &usr1, NULL);

    struct sigaction usr2 = {
        .__sigaction_handler = &Usr2Handler,
        .sa_flags = 0,
        .sa_mask = 0};
    sigaction (SIGUSR2, &usr2, NULL);

    while (1)
        sleep (1);
}

void Usr1Handler (int signal)
{
    printf ("London ");
    fflush (stdout);
    return;
}

void Usr2Handler (int signal)
{
    printf ("is the capital of Great Britain.\n");
    fflush (stdout);
    return;
}
