#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>

/* Создайте набор из 16 семафоров SysV с ключем:
   key_t key = ftok("/tmp/sem.temp", 1);
   
   Проинициализируйте их порядковыми номерами [0..15]. */

#if _SEM_SEMUN_UNDEFINED
union semun {
    int val;                   // value for SETVAL
    struct semid_ds *buf;      // buffer for IPC_STAT & IPC_SET
    unsigned short int *array; // array for GETALL & SETALL
    struct seminfo *__buf;     // buffer for IPC_INFO
};
#endif

int main(int argc, char **argv)
{
    const char *pathName = "/tmp/sem8.temp";    // file must exists
    key_t key;
    if ((key = ftok(pathName, 1)) == -1) {
        perror("ftok failure");
        return -1;
    } else {
        printf("ftok key=%d\n", key);
    }

    int nsems = 16;
    int semflg = 0666 | IPC_CREAT; //создать группу семафоров // IPC_EXCL; // использовать уже существующую группу

    int semid;
    if (semid = semget(key, nsems, semflg) == -1) {
        perror("semget failure");
        return -1;
    } else {
        printf("semget semid=%d\n", semid);
    }

    /*
    struct semid_ds sds;
    sds.sem_perm.uid = getuid();
    sds.sem_perm.gid = getgid();
    sds.sem_perm.mode = 0666; // 9ть младших бит
    union semun su1;
    su1.buf = &sds;
    if (semctl(semid, 0, IPC_SET, su1) == -1) {
        perror("semctl IPC_SET failure");
        return -1;
    } else {
        printf("semctl IPC_SET success\n");
    }*/

    union semun su2;
    for (int i = 0; i < nsems; i++) {
        su2.val = i;
        if (semctl(semid, i, SETVAL, su2) == -1) {
            perror("semctl SETVAL failure");
            return -1;
        } else {
            printf("semctl SETVAL success, i=%d\n", i);
        }
        
    }

    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID failure");
        return -1;
    } else {
        printf("semctl IPC_RMID success\n");
    }

    return 0;
}
