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
#define BUF_LEN 20
#define URG_THRES 80

/* Global data buffers */
long long urgent[BUF_LEN], normal[BUF_LEN];

/* Global semaphores */
sem_t *emptyn, *emptyu, *fulln, *fullu;

long long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void *producer(void *data)
{
    int bufsel, cntn, cntu, posn, posu;
    long long tstamp;
    struct timespec sleep_timespec;

    sleep_timespec.tv_sec = 0;

    for (cntn = 0, cntu = 0, posn = 0, posu = 0; cntn + cntu < DATA_LEN; )
    {
        /* Sleep 1-10 milliseconds */
        sleep_timespec.tv_nsec = (1 + rand() % 10) * 1000000;
        nanosleep(&sleep_timespec, NULL);

        /* Get current timestamp and an integer in 0-99 */
        tstamp = current_timestamp();
        bufsel = rand() % 100;

        /* Select the buffer to use */
        if (bufsel < URG_THRES)
        {
            /* Print urgent tstamp */
            printf("Putting %llu in buffer normal.\n", tstamp);

            /* Wait on normal empty */
            while (sem_wait(emptyn) == -1);

            /* Put tstamp in normal */
            normal[posn] = tstamp;

            /* Post on normal full */
            while (sem_post(fulln) == -1);

            /* Update posn and cntn */
            posn = (posn + 1) % BUF_LEN;
            cntn++;
        }
        else
        {
            /* Print normal tstamp */
            printf("Putting %llu in buffer urgent.\n", tstamp);

            /* Wait on urgent empty */
            while (sem_wait(emptyu) == -1);

            /* Put tstamp in urgent */
            urgent[posu] = tstamp;

            /* Post on urgent full */
            while (sem_post(fullu) == -1);

            /* Update posu and cntu */
            posu = (posu + 1) % BUF_LEN;
            cntu++;
        }
    }

    printf("Producer completed. Total urgent: %d (%d%%), total normal: %d (%d%%).\n", 
        cntu, cntu * 100 / DATA_LEN, cntn, cntn * 100 / DATA_LEN);
}

void *consumer(void *data)
{
    int cntn, cntu, posn, posu;
    long long tstamp;
    struct timespec sleep_timespec;

    sleep_timespec.tv_sec = 0;
    sleep_timespec.tv_nsec = 10000000;

    for (cntn = 0, cntu = 0, posn = 0, posu = 0; cntn + cntu < DATA_LEN; )
    {
        /* Sleep 10 milliseconds */
        nanosleep(&sleep_timespec, NULL);

        /* Check if data is available on buffers */
        if (sem_trywait(fullu) == 0)
        {
            /* Get tstamp from urgent */
            tstamp = urgent[posu];
            
            /* Post on urgent empty */
            while (sem_post(emptyu) == -1);

            /* Print urgent tstamp */
            printf("Retrieving %llu from buffer urgent.\n", tstamp);
            
            /* Update posu and cntu */
            posu = (posu + 1) % BUF_LEN;
            cntu++;
        }
        else if (sem_trywait(fulln) == 0)
        {
            /* Get tstamp from normal */
            tstamp = normal[posn];
            
            /* Post on normal empty */
            while (sem_post(emptyn) == -1);
            
            /* Print normal tstamp */
            printf("Retrieving %llu from buffer normal.\n", tstamp);

            /* Update posn and cntn */
            posn = (posn + 1) % BUF_LEN;
            cntn++;
        }
    }

    printf("Consumer completed. Total urgent: %d (%d%%), total normal: %d (%d%%).\n", 
        cntu, cntu * 100 / DATA_LEN, cntn, cntn * 100 / DATA_LEN);
}

int main(int argc, char const *argv[])
{
    pthread_t prod, cons;

    /* Initialize random seed */
    srand(time(0));

    /* Allocate semaphores */
    if ((emptyn = malloc(sizeof(sem_t))) == NULL || 
        (emptyu = malloc(sizeof(sem_t))) == NULL || 
        (fulln = malloc(sizeof(sem_t))) == NULL || 
        (fullu = malloc(sizeof(sem_t))) == NULL)
    {
        fprintf(stderr, "Cannot allocate semaphores.\n");
        return -1;
    }

    /* Initialize semaphores */
    if (sem_init(emptyn, 0, BUF_LEN) == -1 ||
        sem_init(emptyu, 0, BUF_LEN) == -1 ||
        sem_init(fulln, 0, 0) == -1 ||
        sem_init(fullu, 0, 0) == -1)
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

    /* Destroy semaphores */
    sem_destroy(emptyn);
    sem_destroy(emptyu);
    sem_destroy(fulln);
    sem_destroy(fullu);

    /* Free semaphores */
    free(emptyn);
    free(emptyu);
    free(fulln);
    free(fullu);

    return 0;
}