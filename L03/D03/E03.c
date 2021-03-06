/******************************************************************************
 * Lab 03 - Exercise 3                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define QUICK_REC 0
#define QUICK_THR 1

/* Global size threshold */
int size;

/* Quicksort data structure */
struct qs_data_s
{
    int *v, left, right;
};

/* Function prototypes */
void swap(int v[], int i, int j);
void *quicksort(void *data);
int quicksort_wrapper(struct qs_data_s *data, pthread_t *tid);

void swap(int v[], int i, int j)
{
    int tmp;

    tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
}

void *quicksort(void *data)
{
    int i, j, x, resl, resr;
    struct qs_data_s data_left, data_right;
    pthread_t tid_left, tid_right;

    /* Retrieve data from the structure */
    int *v = ((struct qs_data_s*) data)->v;
    int left = ((struct qs_data_s*) data)->left;
    int right = ((struct qs_data_s*) data)->right;

    /* Standard quicksort algorithm */
    if (left >= right)
        return NULL;
    
    x = v[left];
    i = left - 1;
    j = right + 1;
    
    while (i < j)
    {
        while (v[--j] > x);
        while (v[++i] < x);
        
        if (i < j)
            swap(v, i, j);
    }

    /* Prepare the data structure for the left part */
    data_left.v = v;
    data_left.left = left;
    data_left.right = j;
    
    /* Prepare the data structure for the right part */
    data_right.v = v;
    data_right.left = j+1;
    data_right.right = right;

    /* Call on the left and right part */
    resl = quicksort_wrapper(&data_left, &tid_left);
    resr = quicksort_wrapper(&data_right, &tid_right);

    /* Join threads if they have been created */
    if (resl == QUICK_THR)
        pthread_join(tid_left, NULL);
    if (resr == QUICK_THR)
        pthread_join(tid_right, NULL);
}

int quicksort_wrapper(struct qs_data_s *data, pthread_t *tid)
{
    if (data->right - data->left < size)
    {
        /* Use recursion */
        printf("R: %d - %d (len: %d)\n", data->left, data->right, data->right - data->left);
        quicksort(data);
        return QUICK_REC;
    }
    else
    {
        /* Use threads */
        printf("T: %d - %d (len: %d)\n", data->left, data->right, data->right - data->left);
        pthread_create(tid, NULL, quicksort, data);
        return QUICK_THR;
    }
}

int main(int argc, char const *argv[])
{
    int i, fd, len;
    struct stat file_stats;
    struct qs_data_s data;
    pthread_t tid;

    /* Check command line and set size threshold */
    if (argc < 3)
    {
        fprintf(stderr, "Not enough input arguments.\n");
        return -1;
    }
    size = atoi(argv[1]);

    /* Open file in read-write mode */
    fd = open(argv[2], O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "Cannot open input file.\n");
        return -1;
    }

    /* Read file stats */
    if (fstat(fd, &file_stats) != 0)
    {
        fprintf(stderr, "Cannot read file stats.\n");
        return -1;
    }
    len = file_stats.st_size / sizeof(int);

    /* Map the file to main memory */
    int *v = mmap(NULL, file_stats.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (v == MAP_FAILED)
    {
        fprintf(stderr, "Cannot map input file to memory.\n");
        return -1;
    }

    /* Setup the data structure for the first thread */
    data.v = v;
    data.left = 0;
    data.right = len;

    /* Call quicksort on the entire array */
    if (quicksort_wrapper(&data, &tid) == QUICK_THR)
        pthread_join(tid, NULL);
    
    /* Print sorted vector */
    printf("Sorting finished. Sorted vector:\n");
    for (i = 0; i < len; i++)
        printf("%d ", v[i]);
    printf("\n");
    
    /* Unmap and close the file */
    munmap(v, file_stats.st_size);
    close(fd);

    return 0;
}
