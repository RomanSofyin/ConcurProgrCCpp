#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>  // open
#include <unistd.h> // write
#include <signal.h>
#include <stdio.h>

/* Создайте два процесса - родитель и потомок
   (сохраните их PIDы в /home/box/pid_parent и /home/box/pid_child).
   Потомок должен убиваться по SIGTERM.
   При убийстве потомка не должно остаться процессов-зомби. */

void writePid(const char *name, pid_t pid)
{
    int fd = open(name, O_WRONLY | O_CREAT, 0);
    write(fd, &pid, sizeof(pid));
    close(fd);
}

int main(int argc, char **argv)
{
    pid_t pid_parent = getpid();
    writePid("./pid_parent", pid_parent);

    pid_t pid_child = fork();
    if (pid_child < 0)
    {
        printf("fork() failed\n");
    }
    else if (pid_child == 0)
    {
        /* child code goes here */
        signal(SIGTERM, SIG_IGN);
        printf("Child process: calling pause()\n");
        pause();
        return 0; // to be sure that parent's code won't be executed
    }
    /* parent code goes here */

    writePid("./pid_child", pid_child);
    int child_rc;
    waitpid(pid_child, &child_rc, 0);   // wait until child completes
    printf("Parent process: child's rc=%d\n",child_rc);
    return 0;
}
