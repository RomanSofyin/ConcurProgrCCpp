#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>  // open
#include <unistd.h> // write
#include <signal.h>
#include <stdio.h>

/* Нет нужды создавать здесь доченый процесс.
   Когда будет время попробовать, вариант без fork() с getpid() и аналогичными signal() вызовами. */

int main(int argc, char **argv) {
    pid_t pid = fork();
    if (pid == -1) {
        printf("fork() failed\n");
    } else if (pid == 0) {
        /* child code goes here */
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        printf("Child process - goes to pause()\n");
        pause();
    } else {
        /* parent code goes here */
        int fd = open( "./pid", O_WRONLY|O_CREAT, 0);
        write(fd, &pid, sizeof(pid));
        close(fd);
        printf("Parent process - completes\n");
    }
    return 0;
}








