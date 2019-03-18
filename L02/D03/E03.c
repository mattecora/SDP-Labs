/******************************************************************************
 * Lab 02 - Exercise 3                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/* Global semaphore */
sem_t *s;

void *thread_runner_1(void *data)
{
    int sleep_time, wait_time;
    struct timespec sleep_timespec, wait_timespec;

    /* Compute random sleep time */
    sleep_time = rand() % 5 + 1;

    /* Convert data to wait time */
    wait_time = *((int*) data);

    /* Initialize timespecs */
    sleep_timespec.tv_sec = sleep_time / 1000;
    sleep_timespec.tv_nsec = (sleep_time - sleep_time /1000 * 1000) * 1000000;
    wait_timespec.tv_sec = time(0) + wait_time / 1000;
    wait_timespec.tv_nsec = (wait_time - wait_time / 1000 * 1000) * 1000000;

    /* Sleep with the given timespec */
    nanosleep(&sleep_timespec, NULL);

    /* Wait on the semaphore with timeout */
    printf("Waiting on semaphore after %d milliseconds.\n", sleep_time);
    switch (sem_timedwait(s, &wait_timespec))
    {
        /* Normal exit */
        case 0:
            printf("Wait returned normally.\n");
            break;
        
        /* Timeout exit */
        case -1:
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
        fprintf(stderr, "Cannot instantiate memory.\n");
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
