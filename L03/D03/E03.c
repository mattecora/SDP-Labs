/******************************************************************************
 * Lab 03 - Exercise 3                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

#define SIZE 100

struct thread_data_s
{
    int *v, left, right;
};

void swap(int v[], int i, int j)
{
    int tmp;

    tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
}

void *quicksort(void *data)
{
    int i, j, x, tmp;
    struct thread_data_s data_left, data_right;
    pthread_t tid_left, tid_right;

    /* Retrieve data from the structure */
    int *v = ((struct thread_data_s*) data)->v;
    int left = ((struct thread_data_s*) data)->left;
    int right = ((struct thread_data_s*) data)->right;

    printf("Thread created, sorting from %d to %d.\n", left, right);

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

    /* Prepare the data structure for the left thread */
    data_left.v = v;
    data_left.left = left;
    data_left.right = j;
    
    /* Prepare the data structure for the right thread */
    data_right.v = v;
    data_right.left = j+1;
    data_right.right = right;

    /* Create and join left and right threads */
    pthread_create(&tid_left, NULL, quicksort, &data_left);
    pthread_create(&tid_right, NULL, quicksort, &data_right);
    
    pthread_join(tid_left, NULL);
    pthread_join(tid_right, NULL);
}

int main(int argc, char const *argv[])
{
    int fd;
    struct thread_data_s data;
    pthread_t tid;

    /* Open file in read-write mode */
    fd = open("data.b", O_RDWR);

    if (fd < 0)
    {
        fprintf(stderr, "Cannot open input file.\n");
        return -1;
    }

    /* Map the file to main memory */
    int *v = mmap(NULL, SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (v == MAP_FAILED)
    {
        fprintf(stderr, "Cannot map input file to memory.\n");
        return -1;
    }

    /* Setup the data structure for the first thread */
    data.v = v;
    data.left = 0;
    data.right = SIZE;

    /* Create and join the first thread */
    pthread_create(&tid, NULL, quicksort, &data);
    pthread_join(tid, NULL);
    
    printf("Sorting finished.\n");
    
    /* Unmap and close the file */
    munmap(v, SIZE * sizeof(int));
    close(fd);

    return 0;
}
