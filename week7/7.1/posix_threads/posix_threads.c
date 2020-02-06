#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

void   writePid(const char *name, pid_t pid);
void * thread_func(void * arg);

/* Напишите многопоточную программу (POSIX Threads).
   Должен быть как минимум:
    o Один поток, ожидающий pthread_join.
   
   Текст программы должен находиться в файле /home/box/main.cpp
   PID запущенного процесса должен быть сохранен в файле /home/box/main.pid */

int main(int argc, char **argv)
{
    writePid("./main.pid", getpid());   // write pid as required

    pthread_t thread;
    int arg = 1234;
    pthread_create(&thread, NULL, thread_func, &arg);
    
    sleep(1);

    printf("main: going to join the thread\n");
    //int res = 0;
    void * p_result;// = (void *) &res;
    pthread_join(thread, &p_result);
    // note the same addresses of the arg printed in thread_func() and p_result printed below
    printf("main: thread_func gets joined; p_result=%p, result=%d\n", p_result, *((int*)p_result));

    return 0;
}

void * thread_func(void * arg) {
    printf("thread_func: gets called; arg=%p, *((int)arg)=%d\n", arg, *((int*)arg));
    int * int_arg = (int *) arg;
    int a = *int_arg;
    for(int i = 0; i < 3; i++) {
        for(int j=0; j < 50000000; j++) {
            a = (a + random()) % 123456;
        }
        sleep(1); // cool CPU a bit
        printf("thread_func: just woken up; i=%d, a=%d\n", i,a);
    }
    *int_arg = a;
    return arg;   // be carefull not to return a local variable here
}

void writePid(const char *name, pid_t pid)
{
    int fd = open(name, O_WRONLY | O_CREAT, 0);
    write(fd, &pid, sizeof(pid));
    close(fd);
}