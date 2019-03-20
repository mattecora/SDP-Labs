/******************************************************************************
 * Lab 02 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 * -------------------------------------------------------------------------- *
 * This solution uses POSIX timers to generate alarms with nanoseconds granu- *
 * larity; please compile using flag -lrt                                     *
 ******************************************************************************/

/* Necessary for timer functions */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

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
    timer_t timer;
    struct sigevent se;
    struct itimerspec its;

    /* Set alarm handler */
    signal(SIGALRM, sig_handler);

    /* Set timer to produce a SIGALRM on timeout */
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;

    /* Create timer */
    if (timer_create(CLOCK_REALTIME, &se, &timer) == -1)
    {
        fprintf(stderr, "Cannot create timer.\n");
        exit(-1);
    }

    /* Set timer to tmax without repetition */
    its.it_value.tv_sec = tmax / 1000;
    its.it_value.tv_nsec = (tmax - its.it_value.tv_sec * 1000) * 1000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    /* Start timer */
    if (timer_settime(timer, 0, &its, NULL) == -1)
    {
        fprintf(stderr, "Cannot start timer.\n");
        exit(-1);
    }

    /* Set return value to normal */
    ret_val = EXIT_NORM;

    /* Wait on the semaphore */
    while (sem_wait(s) == -1);

    /* Destroy timer */
    timer_delete(timer);

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
    sleep_timespec.tv_sec = 0;
    sleep_timespec.tv_nsec = sleep_time * 1000000;

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
