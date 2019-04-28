#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "mmu.h"
#include "proc.h"

struct sem
{
    int alloc, count, qstart, qend;
    struct proc *queue[NPROC];
};

struct
{
    struct spinlock lock;
    struct sem sems[NSEMS];
} semtable;

void semtableinit()
{
    /* Initialize the semtable lock */
    initlock(&semtable.lock, "semtable");
}

int semalloc()
{
    int i;
    struct sem* s;

    /* Acquire the lock on the semaphore table */
    acquire(&semtable.lock);

    /* Loop through the positions in the semaphore table */
    for (i = 0, s = semtable.sems; i < NSEMS; i++, s++)
    {
        /* Check for a free position */
        if (s->alloc == 0)
        {
            /* Mark the position as allocated */
            s->alloc = 1;

            /* Release the lock on the semaphore table */
            release(&semtable.lock);

            /* Return the allocated semaphore structure */
            return i;
        }
    }

    /* Release the lock on the semaphore table */
    release(&semtable.lock);

    /* No free position */
    return -1;
}

struct sem *semget(int sd)
{
    /* Check that the index is valid and the semaphore is allocated */
    if (sd < 0 || sd >= NSEMS || semtable.sems[sd].alloc == 0)
        panic("semget");

    return &(semtable.sems[sd]);
}

void seminit(struct sem *s, int count)
{
    /* Acquire the lock on the semaphore table */
    acquire(&semtable.lock);
    
    /* Set the initial value */
    s->count = count;

    /* Set the queue as initially empty */
    s->qstart = 0;
    s->qend = 0;

    /* Release the lock on the semaphore table */
    release(&semtable.lock);
}

void semdestroy(struct sem *s)
{
    /* Acquire the lock on the semaphore table */
    acquire(&semtable.lock);
    
    /* Mark semaphore as unallocated */
    s->alloc = 0;

    /* Release the lock on the semaphore table */
    release(&semtable.lock);
}

void semwait(struct sem *s)
{
    /* Acquire the lock on the semaphore table */
    acquire(&semtable.lock);
    
    /* Decrement the semaphore count */
    s->count--;

    /* Check the semaphore value */
    while (s->count < 0)
    {
        /* Put the process into the queue */
        s->queue[s->qend] = proc;

        /* Update the queue end */
        s->qend = (s->qend + 1) % NPROC;

        /* Wait on the process queue */
        sleep(proc, &semtable.lock);
    }

    /* Release the lock on the semaphore table */
    release(&semtable.lock);
}

void sempost(struct sem *s)
{
    /* Acquire the lock on the semaphore table */
    acquire(&semtable.lock);

    /* Increment the semaphore count */
    s->count++;
    
    /* Check the semaphore value */
    if (s->count <= 0)
    {
        /* Unlock first thread in queue */
        wakeup(s->queue[s->qstart]);

        /* Update queue start */
        s->qstart = (s->qstart + 1) % NPROC;
    }

    /* Release the lock on the semaphore table */
    release(&semtable.lock);
}