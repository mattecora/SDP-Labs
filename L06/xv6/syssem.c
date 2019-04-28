#include "types.h"
#include "defs.h"
#include "syscall.h"

int sys_sem_alloc(void)
{
    /* Call internal semalloc() */
    return semalloc();
}

int sys_sem_init(void)
{
    int sd, val;

    /* Retrieve the semaphore and the initial value */
    if (argint(0, &sd) < 0 || argint(1, &val) < 0)
        return -1;

    /* Call internal seminit() */
    seminit(semget(sd), val);
    return 0;
}

int sys_sem_destroy(void)
{
    int sd;

    /* Retrieve the semaphore */
    if (argint(0, &sd) < 0)
        return -1;
    
    /* Call internal semdestroy() */
    semdestroy(semget(sd));
    return 0;
}

int sys_sem_wait(void)
{
    int sd;

    /* Retrieve the semaphore */
    if (argint(0, &sd) < 0)
        return -1;
    
    /* Call internal semwait() */
    semwait(semget(sd));
    return 0;
}

int sys_sem_post(void)
{
    int sd;

    /* Retrieve the semaphore */
    if (argint(0, &sd) < 0)
        return -1;
    
    /* Call internal sempost() */
    sempost(semget(sd));
    return 0;
}