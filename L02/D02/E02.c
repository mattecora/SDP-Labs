/******************************************************************************
 * Lab 02 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#define EXIT_NORM 0
#define EXIT_TOUT 1

/* Global semaphore */
sem_t *s;

/* Return value for wait_with_timeout */
int ret_val;

void sig_handler(int signal)
{
    if (signal == SIGALRM)
    {
        /* Change return value to timeout */
        ret_val = EXIT_TOUT;

        /* Signal on the semaphore */
        sem_post(s);
    }
}

int wait_with_timeout(sem_t *s, int tmax)
{
    /* Set alarm handler */
    signal(SIGALRM, sig_handler);

    /* Start alarm timer */
    alarm(tmax / 1000);

    /* Set return value to normal */
    ret_val = EXIT_NORM;

    /* Wait on the semaphore */
    sem_wait(s);

    /* Reset the alarm timer */
    alarm(0);

    /* Restore default handler */
    signal(SIGALRM, SIG_DFL);

    return ret_val;
}

void *thread_runner_1(void *data)
{
    int sleep_time;
    struct timespec sleep_timespec;

    /* Compute random sleep time */
    sleep_time = rand() % 5 + 1;

    /* Initialize timespec */
    sleep_timespec.tv_sec = sleep_time / 1000;
    sleep_timespec.tv_nsec = (sleep_time - sleep_timespec.tv_sec * 1000) * 1000000;

    /* Sleep with the given timespec */
    nanosleep(&sleep_timespec, NULL);

    /* Wait on the semaphore with timeout */
    printf("Waiting on semaphore after %d milliseconds.\n", sleep_time);
    switch (wait_with_timeout(s, *((int*) data)))
    {
        /* Normal exit */
        case EXIT_NORM:
            printf("Wait returned normally.\n");
            break;
        
        /* Timeout exit */
        case EXIT_TOUT:
            printf("Wait returned for timeout.\n");
            break;
    }

    return NULL;
}

void *thread_runner_2(void *data)
{
    int sleep_time;
    struct timespec sleep_timespec;

    /* Compute random sleep time */
    sleep_time = rand() % 9000 + 1000;

    /* Initialize timespec */
    sleep_timespec.tv_sec = sleep_time / 1000;
    sleep_timespec.tv_nsec = (sleep_time - sleep_timespec.tv_sec * 1000) * 1000000;

    /* Sleep with the given timespec */
    nanosleep(&sleep_timespec, NULL);

    /* Signal on the semaphore */
    printf("Performing signal on semaphore after %d milliseconds.\n", sleep_time);
    sem_post(s);

    return NULL;
}

int main(int argc, char const *argv[])
{
    int tmax;
    pthread_t tid1, tid2;

    /* Read tmax from command line */
    tmax = atoi(argv[1]);

    /* Initialize random seed */
    srand(time(0));

    /* Initialize semaphore */
    s = malloc(sizeof(sem_t));

    if (s == NULL)
    {
        fprintf(stderr, "Cannot allocate memory.\n");
        return -1;
    }

    sem_init(s, 0, 0);

    /* Create threads */
    pthread_create(&tid1, NULL, thread_runner_1, &tmax);
    pthread_create(&tid2, NULL, thread_runner_2, NULL);

    /* Join threads */
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    /* Destroy semaphore */
    sem_destroy(s);
    free(s);

    return 0;
}
