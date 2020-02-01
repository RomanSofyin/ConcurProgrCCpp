#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

/* Создайте семафор POSIX с именем:
   /test.sem
   
   Проинициализируйте его числом 66. */


int main(int argc, char **argv)
{
    const char *name = "/test.sem";
    
    sem_unlink(name); // just in case
    
    int oflag = O_CREAT;
    mode_t mode = 0666;
    unsigned int value = 66;    // init semaphore value
    sem_t * p_sem;
    if ((p_sem = sem_open(name, oflag, mode, value)) == SEM_FAILED) {
        perror("sem_open failure");
        return -1;
    } else {
        printf("sem_open succeeded, p_sem=%p\n", p_sem);
    }

    int sval = 0;
    if (sem_getvalue(p_sem, &sval) == -1) {
        perror("sem_getvalue failure");
        return -1;
    } else {
        printf("sem_getvalue succeeded, sval=%d\n", sval);
    }

    if (sem_close(p_sem) == -1) {
        perror("sem_close failure");
        return -1;
    } else {
        printf("sem_close succeeded\n");
    }

    return 0;
}
