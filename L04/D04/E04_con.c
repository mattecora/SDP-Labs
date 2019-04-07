/******************************************************************************
 * Lab 04 - Exercise 4 (concurrent version)                                   *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

/* Global variables */
int k, left;
double *v, *v1, *v2, **m;
pthread_mutex_t mutex;

void print_vec(double *v, int n)
{
    int i;

    printf("[ ");
    for (i = 0; i < n; i++)
        printf("%+.4f ", v[i]);
    printf("]");
}

void print_mat(double **m, int n)
{
    int i;

    for (i = 0; i < n; i++)
    {
        print_vec(m[i], n);
        printf("\n");
    }
}

double scalar_prod(double *v1, double *v2, int n)
{
    int i;
    double res = 0;

    for (i = 0; i < n; i++)
        res = res + v1[i] * v2[i];

    print_vec(v1, k);
    printf(" * ");
    print_vec(v2, k);
    printf("' = %+.4f\n", res);
    
    return res;
}

void *scalar_prod_thr(void *data)
{
    long i = (long) data;

    /* Compute v[i] = m[i] * v2 in concurrency */
    v[i] = scalar_prod(m[i], v2, k);
    
    /* Enter the critical section */
    pthread_mutex_lock(&mutex);

    /* Check if the last thread */
    left--;
    if (left == 0)
    {
        /* Compute r = v1 * v */
        printf("Result: %+.4f\n", scalar_prod(v1, v, k));
    }

    /* Exit the critical section */
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char const *argv[])
{
    long i, j;
    pthread_t *tids;

    /* Check and parse input parameters */
    if (argc < 2)
    {
        fprintf(stderr, "Not enough input arguments.\n");
        return -1;
    }
    k = atoi(argv[1]);

    /* Initialize synchronization structures */
    left = k;
    pthread_mutex_init(&mutex, NULL);

    /* Allocate dynamic memory */
    tids = malloc(k * sizeof(pthread_t));
    v = malloc(k * sizeof(double));
    v1 = malloc(k * sizeof(double));
    v2 = malloc(k * sizeof(double));
    m = malloc(k * sizeof(double*));
    
    if (tids == NULL || v == NULL || v1 == NULL || v2 == NULL || m == NULL)
    {
        fprintf(stderr, "Cannot allocate memory.\n");
        return -1;
    }

    for (i = 0; i < k; i++)
    {
        m[i] = malloc(k * sizeof(double));
        if (m[i] == NULL)
        {
            fprintf(stderr, "Cannot allocate memory.\n");
            return -1;
        }
    }
    
    /* Fill in vectors and matrix */
    srand(time(0));
    for (i = 0; i < k; i++)
    {
        v1[i] = (double)rand() / RAND_MAX - 0.5;
        v2[i] = (double)rand() / RAND_MAX - 0.5;

        for (j = 0; j < k; j++)
            m[i][j] = (double)rand() / RAND_MAX - 0.5;
    }

    /* Print vectors and matrix */
    printf("v1:\n");
    print_vec(v1, k);
    printf("\nv2:\n");
    print_vec(v2, k);
    printf("\nm:\n");
    print_mat(m, k);
    printf("\n");

    /* Create threads */
    for (i = 0; i < k; i++)
    {
        /* Using long to avoid warnings when casting i to a pointer on x64 */
        if (pthread_create(&tids[i], NULL, scalar_prod_thr, (void*) i) != 0)
        {
            fprintf(stderr, "Cannot create threads.\n");
            return -1;
        }
    }

    /* Join threads */
    for (i = 0; i < k; i++)
    {
        if (pthread_join(tids[i], NULL) != 0)
        {
            fprintf(stderr, "Cannot join threads.\n");
            return -1;
        }
    }

    /* Free dynamic memory */
    for (i = 0; i < k; i++)
        free(m[i]);
    
    free(m);
    free(v2);
    free(v1);
    free(v);
    free(tids);

    return 0;
}