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
csem_t *empty, *full;

void *producer(void *data)
{
    int i, wpos = 0;

    for (i = 0; i < DATA_LEN; i++)
    {
        /* Wait on empty */
        csem_wait(empty);

        /* Produce a value */
        buffer[wpos] = rand() % DATA_LEN;

        /* Update wpos */
        wpos = (wpos + 1) % BUF_LEN;

        /* Post on full */
        csem_post(full);
    }
}

void *consumer(void *data)
{
    int i, rpos = 0;

    for (i = 0; i < DATA_LEN; i++)
    {
        /* Wait on full */
        csem_wait(full);

        /* Consume a value */
        printf("%4d ", buffer[rpos]);

        /* Update rpos */
        rpos = (rpos + 1) % BUF_LEN;

        /* Post on empty */
        csem_post(empty);
    }
}

int main(int argc, char const *argv[])
{
    int i;
    pthread_t prod, cons;

    /* Initialize random seed */
    srand(time(0));
    
    /* Allocate and initialize semaphores */
    empty = malloc(sizeof(csem_t));
    full = malloc(sizeof(csem_t));

    csem_init(empty, BUF_LEN);
    csem_init(full, 0);

    /* Create and join producer and consumer */
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("\n");

    /* Destroy and free semaphores */
    csem_destroy(full);
    csem_destroy(empty);

    free(full);
    free(empty);

    return 0;
}