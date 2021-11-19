#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    double *old_val, low_lim, step; 
    int n_step;
} GetIntInfo;

void* GetInt (void* vinfo);
double func (double x);

// get int x / sin x from pi / 4 to pi / 2
int main (int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf (stderr, "Bad num of arguments.\n");
        return 0;
    }

    int n_thread = atoi (argv[1]),
        n_point  = atoi (argv[2]);

    if (n_point <= 0 || n_thread <= 0)
    {
        fprintf (stderr, "Bad arguments: wrong numbers.\n");
        return 0;
    }

    if (n_point < n_thread)
        n_thread = n_point;

    double integral = 0.0,
           low_lim  = M_PI_4,
           step     = M_PI_4 / n_point;

    int n_step = n_point / n_thread;
    pthread_t* threads = calloc (n_thread, sizeof (*threads));
    if (!threads)
    {
        fprintf (stderr, "Memory allocation error.\n");
        return 0;
    }

    /* typedef struct
    {
        double *old_val, low_lim, step; 
        int n_step;
    } GetIntInfo;
 */
    GetIntInfo* infos = calloc (n_thread, sizeof (*infos));// {&integral, M_PI_4, M_PI_4 / n_point, n_point / n_thread};
    if (!infos)
    {
        fprintf (stderr, "Memory allocation error.\n");
        return 0;
    }
    pthread_mutex_init (&mutex, NULL);

    for (int i_thr = 0; i_thr < n_thread - 1; i_thr++)
    {
        infos[i_thr] = (GetIntInfo) {&integral, low_lim, step, n_step};
        pthread_create (threads + i_thr, NULL, GetInt, infos + i_thr);
        low_lim += step * n_step;
        #ifdef DEBUG
            printf ("%d created.\n", i_thr);
        #endif
    }
    
    infos[n_thread - 1] = (GetIntInfo) {&integral, low_lim, step, n_step + n_point % n_thread};
    pthread_create (threads + n_thread - 1, NULL, GetInt, infos + n_thread -1);

    void* get_ret = NULL;
    for (int i_thr = 0; i_thr < n_thread; i_thr++)
    {
        pthread_join (threads[i_thr], &get_ret);
        #ifdef DEBUG
            printf ("%d ended.\n", i_thr);
        #endif
    }

    pthread_mutex_destroy (&mutex);
    printf ("%.15lf\n", integral);
    return 0;
}

void* GetInt (void* vinfo)
{
    GetIntInfo* info = (GetIntInfo*) vinfo;

    double res = 0.0;
    for (int i_step = 0; i_step < info->n_step; i_step++)
        res += info->step * func (info->low_lim + info->step * i_step);

    pthread_mutex_lock (&mutex);
    *(info->old_val) += res;
    pthread_mutex_unlock (&mutex);

    pthread_exit (NULL);
    return NULL;
}

double func (double x)
{
    return x / sin (x);
}
