#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

enum Semaphores
{
    SEM_CRIT_ZONE  = 0,
    SEM_IS_CREATED = 1,
};

enum Sizes
{
    STK_SIZE     = 0,
    STK_CAPACITY = 1,
    
    STK_SHIFT
};

typedef struct
{
    size_t* data;
    int shmid, semid;
    // semid: 0: critical zone, 
    //        1: status: 0 - not created
    //                   1 - created
    //        2: size
    //        3: capacity
} stack_t;

static int GetShm (stack_t* stack, key_t key, int size, int flag);

stack_t* attach_stack (key_t key, int size) // ToDo: COPYPASTE!!!! VERY BIG FUNCTION!
{
    stack_t* stack = calloc (1, sizeof (*stack));
    if (!stack)
    {
        perror ("Calloc error");
        return NULL;
    }

    stack->semid = semget (key, 3, IPC_CREAT | IPC_EXCL | 0666);
    if (stack->semid == -1)
    {
        errno = 0;
        stack->semid = semget (key, 3, 0666);
        if (stack->semid == -1)
        {
            perror ("Semaphore error");
            free (stack);
            return NULL;
        }

        struct sembuf sops = {SEM_IS_CREATED, -1, 0};
        if (semop (stack->semid, &sops, 1))
        {
            perror ("Semaphore error");
            free (stack);
            return NULL;
        }

        if (GetShm (stack, key, size, 0666))
        {
            free (stack);
            return NULL;
        }

        sops.sem_num = 1;
        semop (stack->semid, &sops, 1);
    }

    else
    {
        if (GetShm (stack, key, size, 0666 | IPC_CREAT))
        {
            free (stack);
            return NULL;
        }

        struct sembuf sops[2] = {{SEM_IS_CREATED,  1, 0},
                                 {SEM_CRIT_ZONE,   1, 0},};
        semop (stack->semid, sops, 2);
    }

    return stack;
}

static int GetShm (stack_t* stack, key_t key, int size, int flag)
{
    stack->shmid = shmget (key, (size + 2) * sizeof (*(stack->data)), flag);
    if (stack->shmid == -1)
    {
        perror ("Shmget error");
        return -1;
    }

    stack->data = shmat (stack->shmid, NULL, SHM_RDONLY);
    if (stack->data == (void*) -1)
    {
        perror ("Shmat error");
        return -1;
    }

    return 0;
}

/* Detaches existing stack from process. 
Operations on detached stack are not permitted since stack pointer becomes invalid. */
int detach_stack (stack_t* stack)
{
    struct sembuf sops = {SEM_IS_CREATED, -1, 0};
    semop (stack->semid, &sops, 1);

    struct shmid_ds buf = {};
    shmctl (stack->shmid, IPC_STAT, &buf);

    if (buf.shm_nattch == 1 && (buf.shm_perm.mode & SHM_DEST))
    {
        semctl (stack->semid, 0, IPC_RMID);
        shmdt (stack->data);
    }
    else
    {
        shmdt (stack->data);
        sops.sem_op = 1;
        semop (stack->semid, &sops, 1);
    }

    stack->data = NULL;
    stack->semid = -1;
    stack->shmid = -1;
    free (stack);
    return 0; // ToDo: check errors
}

/* Marks stack to be destroed. Destruction are done after all detaches */ 
int mark_destruct (stack_t* stack)
{
    return shmctl (stack->shmid, IPC_RMID, NULL);
}

/* Returns stack maximum size. */
int get_size (stack_t* stack)
{
    return stack->data[STK_CAPACITY];
}

/* Returns current stack size. */
int get_count (stack_t* stack)
{
    return stack->data[STK_SIZE];
}

/* Push val into stack. */
int push (stack_t* stack, size_t val)
{
    int ret_val = 0;
    struct sembuf sops = {SEM_CRIT_ZONE, -1, 0}; // ToDo: function
    semop (stack->semid, &sops, 1);

    int size     = get_count (stack);
    int capacity = get_size  (stack);

    if (0 <= size && size < capacity)
    {
        stack->data[size + STK_SHIFT] = val;
        stack->data[STK_SIZE]++;
    }
    else
        ret_val = -1;

    sops.sem_op = 1;
    semop (stack->semid, &sops, 1); // ToDo: function
    return ret_val;
}

/* Pop val from stack into memory */
int pop (stack_t* stack, size_t* val)
{
    int ret_val = 0;
    struct sembuf sops = {SEM_CRIT_ZONE, -1, 0};
    semop (stack->semid, &sops, 1);

    int size = get_size (stack);
    if (size > 0)
    {
        *val = stack->data[size + STK_SHIFT - 1];
        stack->data[STK_SIZE]--;
    }
    else
        ret_val = -1;

    sops.sem_op = 1;
    semop (stack->semid, &sops, 1);
    return ret_val;
}

//---------------------------------------------
/* Additional tasks */

/* Control timeout on push and pop operations in case stack is full or empty.
val == -1 Operations return immediatly, probably with errors.
val == 0  Operations wait infinitely.
val == 1  Operations wait timeout time.
*/
int set_wait (int val, struct timespec* timeout);
