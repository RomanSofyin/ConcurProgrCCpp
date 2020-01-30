#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

/* Создайте очередь сообщения SysV с ключем:
   key_t key = ftok("/tmp/msg.temp", 1);
   
   Тестовая среда пошлет в нее сообщение.
   struct message {
       long mtype;
       char mtext[80];
   }

   Получите его и сохраните в /home/box/message.txt */

struct message
{
    long mtype;
    char mtext[80];
};

int main(int argc, char **argv)
{
    struct message msg;
    const char * pathName = "/tmp/msg.temp";
    const char * fileName = "/home/box/message.txt";

    key_t key = ftok(pathName, 1);
    int mq_id = msgget(key, (IPC_CREAT | 0644));

    int n_msgrcv = msgrcv(mq_id, &msg, sizeof(msg.mtext), 
        0,                              // тип принимаемого сообщения неважен
        MSG_NOERROR | IPC_NOWAIT);      // получить столько сколько поместится + не блокировать процесс если сообщений нет

    msgctl(mq_id, IPC_RMID, NULL);

    int fd = open(fileName, O_WRONLY);
    ssize_t n_write = write(fd, msg.mtext, n_msgrcv);

    return 0;
}
