#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void    sigint_handler(int var);
void    writePid(const char *name, pid_t pid);
void*   thread_cond(void * arg);
void*   thread_barrier(void * arg);

struct s_cond_data {
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
} cond_data;

struct s_locks {
    s_cond_data         cond_data;
    pthread_barrier_t   barrier;
} locks;

/* Напишите многопоточную программу (POSIX Threads).
   Должны быть как минимум:
    o Один поток, ожидающий события по условной переменной.
    o Один поток, ожидающий достижения барьера.
   
   Текст программы должен находиться в файле /home/box/main.cpp
   PID запущенного процесса должен быть сохранен в файле /home/box/main.pid */

int main(int argc, char **argv)
{
    writePid("./main.pid", getpid());   // write pid as required

    struct s_locks locks = {{PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER}, 0};
    pthread_barrier_init(&locks.barrier, NULL, 2);  // num of threads locked on a barrier - 2

    pthread_t t_cond, t_barrier;

    printf("main: creating the thread_cond\n");
    pthread_create(&t_cond, NULL, thread_cond, &locks.cond_data);     // create a thread then
    
    printf("main: creating the thread_barrier\n");
    pthread_create(&t_barrier, NULL, thread_barrier, &locks.barrier);
    
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    printf("main: all threads have been created\n");

    signal(SIGINT, sigint_handler);
    pause();    // sleep until a signal is received

    printf("main: unlocking the threads\n");

    pthread_cond_signal(&locks.cond_data.cond);
    pthread_barrier_wait(&locks.barrier); // this unlocks current and child threads

    pthread_cond_destroy(&locks.cond_data.cond);
    pthread_mutex_destroy(&locks.cond_data.mutex);
    pthread_barrier_destroy(&locks.barrier);

    printf("main: exiting\n");

    return 0;
}

void sigint_handler(int var) {
    printf("\nsigint_handler: we have got a SIGINT\n");
}

void writePid(const char *name, pid_t pid) {
    int fd = open(name, O_WRONLY | O_CREAT, 0);
    write(fd, &pid, sizeof(pid));
    close(fd);
}

void * thread_cond(void * arg) {
    printf("thread_cond: gets called; arg=%p\n", arg);
    s_cond_data * p_cond_data = (s_cond_data *) arg;
    pthread_cond_wait(&p_cond_data->cond, &p_cond_data->mutex);
    printf("thread_cond: returning control\n");
    return 0;
}

void * thread_barrier(void * arg) {
    printf("thread_barrier: gets called; arg=%p\n", arg);
    pthread_barrier_t * p_barrier = (pthread_barrier_t *) arg;
    pthread_barrier_wait(p_barrier);
    printf("thread_barrier: returning control\n");
    return 0;
}

