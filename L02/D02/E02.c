/******************************************************************************
 * Lab 02 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 * -------------------------------------------------------------------------- *
 * This solution uses POSIX timers to generate alarms with nanoseconds granu- *
 * larity; please compile using flag -lrt                                     *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#include "wait_with_timeout.h"

/* Definition of the thread data structure */
struct thread_data_s
{
    int tmax;
    sem_t *s;
};

void *thread_runner_1(void *data)
{
    int sleep_time;
    struct timespec sleep_timespec;

    /* Retrieve thread data */
    sem_t *s = ((struct thread_data_s *) data)->s;
    int tmax = ((struct thread_data_s *) data)->tmax;

    /* Compute random sleep time */
    sleep_time = rand() % 5 + 1;

    /* Initialize timespec */
    sleep_timespec.tv_sec = 0;
    sleep_timespec.tv_nsec = sleep_time * 1000000;

    /* Sleep with the given timespec */
    nanosleep(&sleep_timespec, NULL);

    /* Wait on the semaphore with timeout */
    printf("Waiting on semaphore after %d milliseconds.\n", sleep_time);
    switch (wait_with_timeout(s, tmax))
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

    /* Retrieve thread data */
    sem_t *s = ((struct thread_data_s *) data)->s;

    /* Compute random sleep time */
    sleep_time = rand() % 9001 + 1000;

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
    sem_t *s;
    pthread_t tid1, tid2;
    struct thread_data_s thread_data;

    /* Check number of parameters */
    if (argc < 2)
    {
        fprintf(stderr, "Please specify an input parameter.\n");
        return -1;
    }

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

    if (sem_init(s, 0, 0) == -1)
    {
        fprintf(stderr, "Cannot instantiate semaphores.\n");
        return -1;
    }

    /* Initialize thread data */
    thread_data.s = s;
    thread_data.tmax = tmax;

    /* Create threads */
    if (pthread_create(&tid1, NULL, thread_runner_1, &thread_data) != 0 ||
        pthread_create(&tid2, NULL, thread_runner_2, &thread_data))
    {
        fprintf(stderr, "Cannot create threads.\n");
        return -1;
    }

    /* Join threads */
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    /* Destroy semaphore */
    sem_destroy(s);
    free(s);

    return 0;
}
