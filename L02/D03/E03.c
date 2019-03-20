/******************************************************************************
 * Lab 02 - Exercise 3                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

/* Necessary for clock_gettime */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

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

    /* Initialize sleep timespec */
    sleep_timespec.tv_sec = 0;
    sleep_timespec.tv_nsec = sleep_time * 1000000;
    
    /* Initialize wait timespec (current time + displacement) */
    if (clock_gettime(CLOCK_REALTIME, &wait_timespec) == -1)
    {
        fprintf(stderr, "Cannot get current time.\n");
        exit(-1);
    }

    wait_timespec.tv_sec += wait_time / 1000;
    wait_timespec.tv_nsec += (wait_time - wait_time / 1000 * 1000) * 1000000;
    
    if (wait_timespec.tv_nsec >= 1000000000)
    {
        wait_timespec.tv_sec++;
        wait_timespec.tv_nsec -= 1000000000;
    }

    /* Sleep with the given timespec */
    nanosleep(&sleep_timespec, NULL);

    /* Wait on the semaphore with timeout */
    printf("Waiting on semaphore after %d milliseconds.\n", sleep_time);
    
    while (sem_timedwait(s, &wait_timespec) == -1)
    {
        /* Timeout exit */
        if (errno == ETIMEDOUT)
        {
            printf("Wait returned for timeout.\n");
            return NULL;
        }
    }

    /* Normal exit */
    printf("Wait returned normally.\n");
    return NULL;
}

void *thread_runner_2(void *data)
{
    int sleep_time;
    struct timespec sleep_timespec;

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

    if (sem_init(s, 0, 0) == -1)
    {
        fprintf(stderr, "Cannot instantiate semaphores.\n");
        return -1;
    }

    /* Create threads */
    if (pthread_create(&tid1, NULL, thread_runner_1, &tmax) != 0 || 
        pthread_create(&tid2, NULL, thread_runner_2, NULL))
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
