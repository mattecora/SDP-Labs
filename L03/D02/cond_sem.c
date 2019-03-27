#include "cond_sem.h"

void cond_sem_init(cond_sem_t *cond_sem, int initial_val)
{
    cond_sem->value = initial_val;
    pthread_mutex_init(&cond_sem->lock, NULL);
    pthread_cond_init(&cond_sem->cond, NULL);
}

void cond_sem_wait(cond_sem_t *cond_sem)
{
    pthread_mutex_lock(&cond_sem->lock);
    
    while (cond_sem->value == 0)
        pthread_cond_wait(&cond_sem->cond, &cond_sem->lock);
    
    cond_sem->value--;
    pthread_mutex_unlock(&cond_sem->lock);
}

void cond_sem_post(cond_sem_t *cond_sem)
{
    pthread_mutex_lock(&cond_sem->lock);
    cond_sem->value++;
    pthread_cond_signal(&cond_sem->cond);
    pthread_mutex_unlock(&cond_sem->lock);
}

void cond_sem_destroy(cond_sem_t *cond_sem)
{
    pthread_mutex_destroy(&cond_sem->lock);
    pthread_cond_destroy(&cond_sem->cond);
}