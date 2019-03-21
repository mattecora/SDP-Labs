#include "wait_with_timeout.h"

/* Global semaphore */
sem_t *sem_global;

/* Return value */
int ret_val;

void sig_handler(int signal)
{
    if (signal == SIGALRM)
    {
        /* Change return value to timeout */
        ret_val = EXIT_TOUT;

        /* Signal on the semaphore */
        sem_post(sem_global);
    }
}

int wait_with_timeout(sem_t *s, int tmax)
{
    timer_t timer;
    struct sigevent se;
    struct itimerspec its;

    /* Set the global semaphore */
    sem_global = s;

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