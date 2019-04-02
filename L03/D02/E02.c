/******************************************************************************
 * Lab 03 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "cond_sem.h"

#define DATA_LEN 10000
#define BUF_LEN 20

/* Global data buffer */
int buffer[BUF_LEN];

/* Global semaphores */
sema_t *empty, *full;

void *producer(void *data)
{
    int i, wpos = 0;

    for (i = 0; i < DATA_LEN; i++)
    {
        /* Wait on empty */
        while (sema_wait(empty) == -1);

        /* Produce a value */
        buffer[wpos] = rand() % DATA_LEN;

        /* Post on full */
        while (sema_post(full) == -1);

        /* Update wpos */
        wpos = (wpos + 1) % BUF_LEN;
    }
}

void *consumer(void *data)
{
    int i, rpos = 0;

    for (i = 0; i < DATA_LEN; i++)
    {
        /* Wait on full */
        while (sema_wait(full) == -1);

        /* Consume a value */
        printf("%4d ", buffer[rpos]);

        /* Post on empty */
        while (sema_post(empty) == -1);

        /* Update rpos */
        rpos = (rpos + 1) % BUF_LEN;
    }
}

int main(int argc, char const *argv[])
{
    int i;
    pthread_t prod, cons;

    /* Initialize random seed */
    srand(time(0));
    
    /* Allocate semaphores */
    if ((empty = malloc(sizeof(sema_t))) == NULL || 
        (full = malloc(sizeof(sema_t))) == NULL)
    {
        fprintf(stderr, "Cannot allocate semaphores.\n");
        return -1;
    }

    /* Initialize semaphores */
    if (sema_init(empty, BUF_LEN) == -1 || sema_init(full, 0) == -1)
    {
        fprintf(stderr, "Cannot initialize semaphores.\n");
        return -1;
    }

    /* Create producer and consumer */
    if (pthread_create(&prod, NULL, producer, NULL) != 0 ||
        pthread_create(&cons, NULL, consumer, NULL) != 0)
    {
        fprintf(stderr, "Cannot create threads.\n");
        return -1;
    }

    /* Join producer and consumer */
    if (pthread_join(prod, NULL) != 0 || pthread_join(cons, NULL) != 0)
    {
        fprintf(stderr, "Cannot join threads.\n");
        return -1;
    }

    printf("\n");

    /* Destroy semaphores */
    sema_destroy(full);
    sema_destroy(empty);

    /* Free semaphores */
    free(full);
    free(empty);

    return 0;
}