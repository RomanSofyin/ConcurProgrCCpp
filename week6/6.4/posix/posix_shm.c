#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Создайте общую память POSIX с именем:
   /test.shm
   
   Размер сегмента должен быть 1 мегабайт. Заполните каждый байт числом 13. */

#define MB          1024*1024
#define N_MB        1
#define SHM_SEG_LEN N_MB * MB 

int main(int argc, char **argv)
{
    const char * name   = "/test.shm";
    int fflags          = O_CREAT | O_RDWR | O_TRUNC;
    mode_t fmode        = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // 0666
    int fd;

    if ((fd = shm_open(name, fflags, fmode)) == -1) {
        perror("shm_open failure");
        return -1;
    } else {
        printf("shm_open succeeded, fd=%d\n", fd);
    }
 
    int mprot   = PROT_READ | PROT_WRITE;
    int mflags  = MAP_SHARED;
    void * p_shm;
    
    // Getting the error here: "mmap failure: Exec format error"
    // Possibly a WSL issue, google to sort out.
    if ((p_shm = mmap( NULL /*(void*) (4096 * 4)*/, SHM_SEG_LEN, mprot, mflags, fd, 0)) == MAP_FAILED) {
        perror("mmap failure");
        return -1;
    } else {
        printf("mmap succeeded, p_shm=%p\n", p_shm);
    }

    /* здесь мы пишем в Shared Memory Segment */
    char * p_shm_char;
    char byte = 13;
    for (size_t i = 0; i < SHM_SEG_LEN; i++) {
        p_shm_char = (char *) (p_shm + i);
        *p_shm_char = byte;
        if (!(i % 256))
            printf("Written to p_shm_int=%p, i=%zu\n", p_shm_char, i);
    }

    if (munmap(p_shm, SHM_SEG_LEN) == -1) {
        perror("munmap failure");
        return -1;
    } else {
        printf("munmap succeeded\n");
    }

    if (shm_unlink(name) == -1) {
        perror("shm_unlink failure");
        return -1;
    } else {
        printf("shm_unlink succeeded\n");
    }

    return 0;
}
