#ifndef COND_SEM
#define COND_SEM

#include <pthread.h>

/* Semaphore implemented via conditions and mutexes */
struct sema_s
{
    int value;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

typedef struct sema_s sema_t;

int sema_init(sema_t *csem, int initial_val);
int sema_wait(sema_t *csem);
int sema_post(sema_t *csem);
int sema_destroy(sema_t *csem);

#endif