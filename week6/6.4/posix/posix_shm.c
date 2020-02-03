#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* Создайте общую память POSIX с именем:
   /test.shm
   
   Размер сегмента должен быть 1 мегабайт. Заполните каждый байт числом 13. */

#define MB          1024*1024
#define N_MB        8

int main(int argc, char **argv)
{
    const char * pathName = "/tmp/mem.temp";

    key_t key;
    if ((key = ftok(pathName, 1)) == -1) {
        perror("ftok failure");
        return -1;
    } else {
        printf("ftok succeeded, key=%d\n", key);
    }

    int shm_id;
    if ((shm_id = shmget(key, N_MB * MB, IPC_CREAT | 0660)) == -1) {
        perror("shmget failure");
        return -1;
    } else {
        printf("shmget succeeded, shm_id=%d\n", shm_id);
    }

    void * p_shm;
    if ((p_shm = shmat(shm_id, NULL, 0)) == (void *) -1) {
        perror("shmat failure");
        return -1;
    } else {
        printf("shmat succeeded, p_shm=%p\n", p_shm);
    }
    
    /* здесь мы пишем в Shared Memory Segment */
    int * p_shm_int;
    for (int i = 0; i < N_MB; i++) {
        p_shm_int = (int *) (p_shm + i * MB);
        *p_shm_int = 42;
        printf("Written to p_shm_int=%p, i=%d\n", p_shm_int, i);
    }

    int rc;
    if ((rc = shmdt(p_shm)) == -1) {
        perror("shmdt failure");
        return -1;
    } else {
        printf("shmdt succeeded, rc=%d\n", rc);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failure");
        return -1;
    } else {
        printf("shmctl IPC_RMID succeeded\n");
    }

    return 0;
}
