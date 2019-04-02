#ifndef COND_SEM
#define COND_SEM

#include <pthread.h>

/* Semaphore implemented via conditions and mutexes */
struct csem_s
{
    int value;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

typedef struct csem_s csem_t;

void csem_init(csem_t *csem, int initial_val);
void csem_wait(csem_t *csem);
void csem_post(csem_t *csem);
void csem_destroy(csem_t *csem);

#endif