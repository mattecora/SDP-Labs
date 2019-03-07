/******************************************************************************
 * Lab 01 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

int limits[2][2] = {10, 100, 21, 101};
char *filenames[2][2] = {"fv1.txt", "fv1.b", "fv2.txt", "fv2.b"};

int int_comparator(const void *a, const void *b)
{
    /* Cast, dereference and subtract */
    return *((int*)a) - *((int*)b);
}

int rand_interval(int min, int max, int interval)
{
    /* Scale the rand interval into the given one */
    return interval * (rand() % ((max - min) / interval + 1)) + min;
}

void handle_vector(int num, int *vec, int len)
{
    int i;
    FILE *fp;
    
    /* Allocate the array */
    vec = malloc(len * sizeof(int));
    if (vec == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory.\n");
        return;
    }

    /* Fill the array */
    for (i = 0; i < len; i++)
        vec[i] = rand_interval(limits[num][0], limits[num][1], 2);
    
    /* Sort the array */
    qsort(vec, len, sizeof(int), int_comparator);

    /* Open the text file */
    fp = fopen(filenames[num][0], "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(vec);
        return;
    }
    
    /* Write the text file */
    for (i = 0; i < len; i++)
        fprintf(fp, "%d ", vec[i]);
    fclose(fp);
    
    /* Open the binary file */
    fp = fopen(filenames[num][1], "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(vec);
        return;
    }
    
    /* Write the binary file */
    fwrite(vec, sizeof(int), len, fp);
    fclose(fp);
    
    /* Free the array */
    free(vec);
}

int main(int argc, char const *argv[])
{
    int i, n1, n2, ec;
    int *v1, *v2;
    pid_t children[2];
    FILE *fp;
    
    /* Initialize random seed */
    srand(time(0));

    /* Check input arguments */
    if (argc < 3)
    {
        fprintf(stderr, "Error: not enough input arguments\n");
        return -1;
    }

    /* Convert input arguments */
    n1 = atoi(argv[1]);
    n2 = atoi(argv[2]);

    /* Loop twice */
    for (i = 0; i < 2; i++)
    {
        /* Fork a process */
        if ((children[i] = fork()) == 0)
        {
            /* Child processes a vector and returns */
            handle_vector(i, i == 0 ? v1 : v2, i == 0 ? n1 : n2);
            return i+1;
        }
    }
    
    /* Loop twice */
    for (i = 0; i < 2; i++)
    {
        /* Wait for a child */
        wait(&ec);

        /* Print termination message */
        if (WIFEXITED(ec))
            printf("Process %d (PID %d) has terminated.\n", WEXITSTATUS(ec), children[WEXITSTATUS(ec)-1]);
    }

    return 0;
}
