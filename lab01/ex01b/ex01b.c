/******************************************************************************
 * Lab 01 - Exercise 1 (version b - using Unix system calls)                  *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

int int_comparator(const void *a, const void *b)
{
    return *((int*)a) - *((int*)b);
}

void handle_v1(int n1)
{
    int i, fd, *v1;
    
    /* Allocate the array */
    v1 = malloc(n1 * sizeof(int));
    if (v1 == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory.\n");
        return;
    }

    /* Fill the array */
    for (i = 0; i < n1; i++)
        v1[i] = 2 * (rand() % 46) + 10;
    
    /* Sort the array */
    qsort(v1, n1, sizeof(int), int_comparator);

    /* Open the binary file */
    fd = open("fv1.b", O_CREAT | O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(v1);
        return;
    }

    /* Write the binary file */
    write(fd, v1, n1*sizeof(int));
    close(fd);

    /* Free the array */
    free(v1);
}

void handle_v2(int n2)
{
    int i, fd, *v2;

    /* Allocate the array */
    v2 = malloc(n2 * sizeof(int));
    if (v2 == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory.\n");
        return;
    }

    /* Fill the array */
    for (i = 0; i < n2; i++)
        v2[i] = 2 * (rand() % 41) + 21;

    /* Sort the array */
    qsort(v2, n2, sizeof(int), int_comparator);
    
    /* Open the binary file */
    fd = open("fv2.b", O_CREAT | O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(v2);
        return;
    }

    /* Write the binary file */
    write(fd, v2, n2*sizeof(int));
    close(fd);

    /* Free the array */
    free(v2);
}

int main(int argc, char const *argv[])
{
    int i, n1, n2;
    
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

    /* Do things */
    handle_v1(n1);
    handle_v2(n2);

    return 0;
}
