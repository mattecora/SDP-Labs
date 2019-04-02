#include "cond_sem.h"

int sema_init(sema_t *sema, int initial_val)
{
    /* Set the initial value */
    sema->value = initial_val;

    /* Initialize mutex and condition */
    if (pthread_mutex_init(&sema->lock, NULL) != 0)
        return -1;
    if (pthread_cond_init(&sema->cond, NULL) != 0)
        return -1;
    
    return 0;
}

int sema_wait(sema_t *sema)
{
    /* Lock the mutex */
    if (pthread_mutex_lock(&sema->lock) != 0)
        return -1;
    
    /* Wait on condition until value is 0 */
    while (sema->value == 0)
        if (pthread_cond_wait(&sema->cond, &sema->lock) != 0)
            return -1;
    
    /* Decrement value */
    sema->value--;

    /* Unlock the mutex */
    if (pthread_mutex_unlock(&sema->lock) != 0)
        return -1;
    
    return 0;
}

int sema_post(sema_t *sema)
{
    /* Lock the mutex */
    if (pthread_mutex_lock(&sema->lock) != 0)
        return -1;

    /* Increment value */
    sema->value++;

    /* Signal on condition if new value is 1 */
    if (sema->value == 1)
        if (pthread_cond_signal(&sema->cond) != 0)
            return -1;

    /* Unlock the mutex */
    if (pthread_mutex_unlock(&sema->lock) != 0)
        return -1;
    
    return 0;
}

int sema_destroy(sema_t *sema)
{
    /* Destroy mutex and condition */
    if (pthread_mutex_destroy(&sema->lock) != 0)
        return -1;
    if (pthread_cond_destroy(&sema->cond) != 0)
        return -1;
    
    return 0;
}