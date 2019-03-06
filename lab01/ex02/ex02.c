/******************************************************************************
 * Lab 01 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <wait.h>

int int_comparator(const void* a, const void* b)
{
    return *((int*)a) - *((int*)b);
}

int handle_v1(int n1)
{
    int i, *v1;
    FILE *fp;
    
    /* Allocate the array */
    v1 = malloc(n1 * sizeof(int));
    if (v1 == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory.\n");
        return -1;
    }

    /* Fill the array */
    for (i = 0; i < n1; i++)
        v1[i] = rand() % 91 + 10;
    
    /* Sort the array */
    qsort(v1, n1, sizeof(int), int_comparator);

    /* Open the text file */
    fp = fopen("fv1.txt", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(v1);
        return -1;
    }
    
    /* Write the text file */
    for (i = 0; i < n1; i++)
        fprintf(fp, "%d ", v1[i]);
    fclose(fp);
    
    /* Open the binary file */
    fp = fopen("fv1.bin", "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(v1);
        return -1;
    }
    
    /* Write the binary file */
    fwrite(v1, sizeof(int), n1, fp);
    fclose(fp);
    
    /* Free the array */
    free(v1);
    return 1;
}

int handle_v2(int n2)
{
    int i, *v2;
    FILE *fp;

    /* Allocate the array */
    v2 = malloc(n2 * sizeof(int));
    if (v2 == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory.\n");
        return -1;
    }

    /* Fill the array */
    for (i = 0; i < n2; i++)
        v2[i] = rand() % 81 + 20;

    /* Sort the array */
    qsort(v2, n2, sizeof(int), int_comparator);
    
    /* Open the text file */
    fp = fopen("fv2.txt", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(v2);
        return -1;
    }

    /* Write the text file */
    for (i = 0; i < n2; i++)
        fprintf(fp, "%d ", v2[i]);
    fclose(fp);

    /* Open the binary file */
    fp = fopen("fv2.bin", "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open output file.\n");
        free(v2);
        return -1;
    }
    
    /* Write the binary file */
    fwrite(v2, sizeof(int), n2, fp);
    fclose(fp);
    
    /* Free the array */
    free(v2);
    return 2;
}

int main(int argc, char const *argv[])
{
    int i, n1, n2;
    int p1, p2, ec;
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

    if ((p1 = fork()) == 0)
    {
        /* Fork and do actions on v1 */
        return handle_v1(n1);
    }
    else if ((p2 = fork()) == 0)
    {
        /* Fork and do actions on v2 */
        return handle_v2(n2);
    }
    
    /* Wait for the first process */
    waitpid(p1, &ec, 0);
    printf("Process 1 (PID %d) has terminated with status code %d.\n", p1, ec);

    /* Wait for the second process */
    waitpid(p2, &ec, 0);
    printf("Process 2 (PID %d) has terminated with status code %d.\n", p2, ec);

    return 0;
}
