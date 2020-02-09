#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void    sigint_handler(int var);
void    writePid(const char *name, pid_t pid);
void*   thread_mutex(void * arg);
void*   thread_spinlock(void * arg);
void*   thread_rdlock(void * arg);
void*   thread_wrlock(void * arg);

struct s_locks {
    pthread_mutex_t     mutex;
    pthread_spinlock_t  spinlock;
    // Note: it should be enought one pthread_rwlock_t here;
    // we need to call pthread_rwlock_wrlock() in main thread first, then both child threads
    // will be waiting when call their respective pthread_rwlock_rdlock/pthread_rwlock_wrlock.
    pthread_rwlock_t    rdlock;
    pthread_rwlock_t    wrlock;
};

/* Напишите многопоточную программу (POSIX Threads).
   Должны быть как минимум:
    o Один поток, ожидающий освобождения мьютекса.
    o Один поток, ожидающий освобождения спин-блокировки.
    o Два потока, ожидающих освобождения RW-блокировки (один должен ожидать чтения, другой - записи).

   Текст программы должен находиться в файле /home/box/main.cpp
   PID запущенного процесса должен быть сохранен в файле /home/box/main.pid */

int main(int argc, char **argv)
{
    writePid("./main.pid", getpid());   // write pid as required

    // static init is more suitable for our purposes
    struct s_locks locks = {PTHREAD_MUTEX_INITIALIZER, 0, PTHREAD_RWLOCK_INITIALIZER, PTHREAD_RWLOCK_INITIALIZER};
    pthread_spin_init(&locks.spinlock, 0);  // spinlock doesn't have a macro to initialize it statically

    /* dynamically initialize the locks from s_locks
    struct s_locks locks;
    pthread_mutex_init(&locks.mutex, NULL);
    pthread_spin_init(&locks.spinlock, 0);
    pthread_rwlock_init(&locks.rdlock, NULL);
    pthread_rwlock_init(&locks.wrlock, NULL); */

    pthread_t t_mutex, t_spinlock, t_rdlock, t_wrlock;

    printf("main: creating the thread_mutex\n");
    pthread_mutex_lock(&locks.mutex);                       // catch the mutex first
    pthread_create(&t_mutex, NULL, thread_mutex, &locks.mutex);     // create a thread then
    
    printf("main: creating the thread_spinlock\n");
    void * p_spinlock = (void *) &locks.spinlock;
    pthread_spin_lock(p_spinlock);
    pthread_create(&t_spinlock, NULL, thread_spinlock, p_spinlock);
    
    printf("main: creating the thread_rdlock\n");
    pthread_rwlock_wrlock(&locks.rdlock);   // we do WRITE lock here to make thread_rdlock waiting on locks.rdlock
    pthread_create(&t_rdlock, NULL, thread_rdlock, &locks.rdlock);    
    
    printf("main: creating the thread_wrlock\n");
    pthread_rwlock_wrlock(&locks.wrlock);
    pthread_create(&t_wrlock, NULL, thread_wrlock, &locks.wrlock);

    printf("main: all threads have been created\n");

    signal(SIGINT, sigint_handler);
    pause();    // sleep until a signal is received

    printf("main: unlocking the threads\n");

    pthread_mutex_unlock(&locks.mutex);
    pthread_spin_unlock(&locks.spinlock);
    pthread_rwlock_unlock(&locks.rdlock);
    pthread_rwlock_unlock(&locks.wrlock);

    // in a good way, it needs to destroy all the locks

    printf("main: exiting\n");

    return 0;
}

void sigint_handler(int var) {
    printf("sigint_handler: we have got a SIGINT\n");
}

void writePid(const char *name, pid_t pid) {
    int fd = open(name, O_WRONLY | O_CREAT, 0);
    write(fd, &pid, sizeof(pid));
    close(fd);
}

void * thread_mutex(void * arg) {
    printf("thread_mutex: gets called; arg=%p\n", arg);
    pthread_mutex_t * p_mutex = arg;
    pthread_mutex_lock(p_mutex);
    printf("thread_mutex: returning control\n");
    return 0;
}

void * thread_spinlock(void * arg) {
    printf("thread_spinlock: gets called; arg=%p\n", arg);
    pthread_spinlock_t * p_spinlock = arg;
    pthread_spin_lock(p_spinlock);
    printf("thread_spinlock: returning control\n");
    return 0;
}

void * thread_rdlock(void * arg) {
    printf("thread_rdlock: gets called; arg=%p\n", arg);
    pthread_rwlock_t * p_rdlocks = arg;
    pthread_rwlock_rdlock(p_rdlocks);
    printf("thread_rdlock: returning control\n");
    return 0;
}

void * thread_wrlock(void * arg) {
    printf("thread_wrlock: gets called; arg=%p\n", arg);
    pthread_rwlock_t * p_rwlocks = arg;
    pthread_rwlock_wrlock(p_rwlocks);
    printf("thread_wrlock: returning control\n");
    return 0;
}
