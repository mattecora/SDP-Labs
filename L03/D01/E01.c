/******************************************************************************
 * Lab 03 - Exercise 1                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define DATA_LEN 10000
#define URG_THRES 80

/* Global data buffers */
long long urgent[DATA_LEN], normal[DATA_LEN];

/* Global semaphores */
sem_t *fulln, *fullu;

long long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void *producer(void *data)
{
    int i, bufsel, posn = 0, posu = 0;
    long long tstamp;
    struct timespec sleep_timespec;

    sleep_timespec.tv_sec = 0;

    for (i = 0; i < DATA_LEN; i++)
    {
        /* Sleep 1-10 milliseconds */
        sleep_timespec.tv_nsec = (1 + rand() % 10) * 1000000;
        nanosleep(&sleep_timespec, NULL);

        /* Get current timestamp and an integer in 0-99 */
        tstamp = current_timestamp();
        bufsel = rand() % 100;

        if (bufsel < URG_THRES)
        {
            /* Use the normal buffer */
            printf("Putting %llu in buffer normal.\n", tstamp);
            normal[posn++] = tstamp;
            sem_post(fulln);
        }
        else
        {
            /* Use the urgent buffer */
            printf("Putting %llu in buffer urgent.\n", tstamp);
            urgent[posu++] = tstamp;
            sem_post(fullu);
        }
    }

    printf("Producer completed. Total urgent: %d, total normal: %d\n", posu, posn);
}

void *consumer(void *data)
{
    int i, posn = 0, posu = 0;
    long long tstamp;
    struct timespec sleep_timespec;

    sleep_timespec.tv_sec = 0;
    sleep_timespec.tv_nsec = 10000000;

    for (i = 0; i < DATA_LEN; )
    {
        /* Sleep 10 milliseconds */
        nanosleep(&sleep_timespec, NULL);

        /* Check if data is available on buffers */
        if (sem_trywait(fullu) == 0)
        {
            /* Process urgent data */
            tstamp = urgent[posu++];
            printf("Retrieving %llu from buffer urgent.\n", tstamp);
            i++;
        }
        else if (sem_trywait(fulln) == 0)
        {
            /* Process normal data */
            tstamp = normal[posn++];
            printf("Retrieving %llu from buffer normal.\n", tstamp);
            i++;
        }
    }

    printf("Consumer completed. Total urgent: %d, total normal: %d\n", posu, posn);
}

int main(int argc, char const *argv[])
{
    pthread_t prod, cons;

    /* Initialize random seed */
    srand(time(0));

    /* Allocate and initialize semaphores */
    fulln = malloc(sizeof(sem_t));
    fullu = malloc(sizeof(sem_t));

    sem_init(fulln, 0, 0);
    sem_init(fullu, 0, 0);

    /* Create and join producer and consumer */
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    /* Destroy and free semaphores */
    sem_destroy(fulln);
    sem_destroy(fullu);

    free(fulln);
    free(fullu);

    return 0;
}