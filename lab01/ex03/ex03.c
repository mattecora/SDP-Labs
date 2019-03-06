/******************************************************************************
 * Lab 01 - Exercise 3                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <pthread.h>

int next, this, last;

void *process_this(void *data)
{
    /* Make this uppercase */
    this = toupper(this);
    return NULL;
}

void *write_last(void *data)
{
    /* Write last to console */
    fputc(last, stdout);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int i;
    FILE *fp;
    pthread_t tid_p, tid_o;

    /* Check input arguments */
    if (argc < 2)
    {
        fprintf(stderr, "Error: not enough input parameters.\n");
        return -1;
    }

    /* Open the input file */
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open input file.\n");
        return -1;
    }

    /* Loop until the end of the file */
    for (i = 0; last != EOF; i++)
    {
        /* Create the processing thread */
        if (i >= 1)
            pthread_create(&tid_p, NULL, process_this, NULL);
        
        /* Create the output thread */
        if (i >= 2)
            pthread_create(&tid_o, NULL, write_last, NULL);

        /* Read the next character */
        next = fgetc(fp);
        
        /* Join the processing thread */
        if (i >= 1)
            pthread_join(tid_p, NULL);
        
        /* Join the output thread */
        if (i >= 2)
            pthread_join(tid_o, NULL);
        
        /* Shift the variables */
        last = this;
        this = next;
    }

    /* Close the input file */
    fclose(fp);

    printf("\nProgram finished: %d characters processed.\n", i-2);
    return 0;
}
