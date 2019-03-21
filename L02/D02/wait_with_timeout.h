#ifndef WAIT_WITH_TIMEOUT
#define WAIT_WITH_TIMEOUT

/* Necessary for timer functions */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#define EXIT_NORM 0
#define EXIT_TOUT 1

int wait_with_timeout(sem_t *s, int tmax);

#endif