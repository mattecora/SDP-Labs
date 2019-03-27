#ifndef COND_PC
#define COND_PC

#include <pthread.h>

#define BUF_LEN 20

struct cond_sem_s
{
    int value;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

typedef struct cond_sem_s cond_sem_t;

void cond_sem_init(cond_sem_t *cond_sem, int initial_val);
void cond_sem_wait(cond_sem_t *cond_sem);
void cond_sem_post(cond_sem_t *cond_sem);
void cond_sem_destroy(cond_sem_t *cond_sem);

#endif