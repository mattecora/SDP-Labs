#include "param.h"
#include "types.h"
#include "user.h"

int stdout = 1;

void st()
{
    int pid;
    int sem1, sem2;

    sem1 = sem_alloc();
    printf(stdout, "sem1: %d\n", sem1);

    sem2 = sem_alloc();
    printf(stdout, "sem2: %d\n", sem2);

    sem_init(sem1, 0);
    sem_init(sem2, 0);
    
    printf(stdout, "A\n");
    pid = fork();
    if (pid)
    {
        sem_wait(sem1);
        printf(stdout, "C\n");
        sem_post(sem2);
        wait();
    }
    else
    {
        sleep(500);
        printf(stdout,  "B\n");
        sem_post(sem1);
        sem_wait(sem2);
        printf(stdout, "D\n");
        exit();
    }

    sem_destroy(sem1);
    sem_destroy(sem2);
}

int main(int argc, char *argv[])
{
    st();
    exit();
}