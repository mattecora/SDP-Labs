#include "cond_sem.h"

void csem_init(csem_t *csem, int initial_val)
{
    /* Set the initial value */
    csem->value = initial_val;

    /* Initialize mutex and condition */
    pthread_mutex_init(&csem->lock, NULL);
    pthread_cond_init(&csem->cond, NULL);
}

void csem_wait(csem_t *csem)
{
    /* Lock the mutex */
    pthread_mutex_lock(&csem->lock);
    
    /* Wait on condition until value is 0 */
    while (csem->value == 0)
        pthread_cond_wait(&csem->cond, &csem->lock);
    
    /* Decrement value */
    csem->value--;

    /* Unlock the mutex */
    pthread_mutex_unlock(&csem->lock);
}

void csem_post(csem_t *csem)
{
    /* Lock the mutex */
    pthread_mutex_lock(&csem->lock);

    /* Increment value */
    csem->value++;

    /* Signal on condition */
    pthread_cond_signal(&csem->cond);

    /* Unlock the mutex */
    pthread_mutex_unlock(&csem->lock);
}

void csem_destroy(csem_t *csem)
{
    /* Destroy mutex and condition */
    pthread_mutex_destroy(&csem->lock);
    pthread_cond_destroy(&csem->cond);
}