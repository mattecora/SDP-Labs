/******************************************************************************
 * Lab 02 - Exercise 1                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

/* Global integer variable */
int g;

/* Global termination flags */
int thread_complete[2] = {0};

/* Global semaphores */
sem_t *sem_server, *sem_client, *sem_print;

/* Definition of the thread data structure */
struct thread_data_s
{
    int id;
    char *filename;
};

void *thread_runner(void *data)
{
    int id, next;
    char *filename;
    FILE *fp;

    /* Retrieve thread data */
    id = ((struct thread_data_s*) data)->id;
    filename = ((struct thread_data_s*) data)->filename;

    /* Open input file */
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot open input file.\n");
        exit(-1);
    }
    
    /* Read next value */
    fread(&next, sizeof(int), 1, fp);

    /* Loop until EOF */
    while (!feof(fp))
    {
        /* Wait on sem_client */
        while (sem_wait(sem_client) == -1);

        /* Copy next value on the global variable */
        g = next;

        /* Signal on sem_server */
        sem_post(sem_server);

        /* Wait on sem_print */
        while (sem_wait(sem_print) == -1);

        /* Print the global variable after processing */
        printf("Thread #%d : %d\n", id, g);

        /* Read next value */
        if (fread(&next, sizeof(int), 1, fp))
        {
            /* Signal on sem_server */
            sem_post(sem_server);
        }
    }

    /* Close input file */
    fclose(fp);

    /* Set complete flag */
    thread_complete[id] = 1;

    /* Signal on sem_server */
    sem_post(sem_server);

    return NULL;
}

int main(int argc, char const *argv[])
{
    int i, total_reqs = 0;
    struct thread_data_s thread_data[2];
    pthread_t tids[2];

    /* Initialize semaphores */
    sem_server = malloc(sizeof(sem_t));
    sem_client = malloc(sizeof(sem_t));
    sem_print = malloc(sizeof(sem_t));

    if (sem_server == NULL || sem_client == NULL || sem_print == NULL)
    {
        fprintf(stderr, "Cannot allocate memory.\n");
        return -1;
    }

    if (sem_init(sem_server, 0, 0) == -1 || 
        sem_init(sem_client, 0, 0) == -1 || 
        sem_init(sem_print, 0, 0) == -1)
    {
        fprintf(stderr, "Cannot instantiate semaphores.\n");
        return -1;
    }

    /* Initialize thread data */
    thread_data[0].id = 0;
    thread_data[1].id = 1;
    thread_data[0].filename = "fv1.b";
    thread_data[1].filename = "fv2.b";

    /* Create threads */
    for (i = 0; i < 2; i++)
    {
        if (pthread_create(&(tids[i]), NULL, thread_runner, (void*) &(thread_data[i])) != 0)
        {
            fprintf(stderr, "Cannot create threads.\n");
            return -1;
        }
    }

    /* Loop until threads are running */
    while (thread_complete[0] == 0 || thread_complete[1] == 0)
    {
        /* Signal on sem_client */
        sem_post(sem_client);
        
        /* Wait on sem_server */
        while (sem_wait(sem_server) == -1);
        
        /* Process global variable */
        g = g * 3;

        /* Increment requests number */
        total_reqs++;

        /* Signal on sem_print */
        sem_post(sem_print);

        /* Wait on sem_server */
        while (sem_wait(sem_server) == -1);
    }

    printf("Total requests : %d\n", total_reqs);

    /* Destroy semaphores */
    sem_destroy(sem_server);
    sem_destroy(sem_client);
    sem_destroy(sem_print);

    free(sem_server);
    free(sem_client);
    free(sem_print);

    return 0;
}
