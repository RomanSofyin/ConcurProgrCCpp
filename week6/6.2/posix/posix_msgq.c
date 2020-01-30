#include <mqueue.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

/* Создайте очередь сообщения POSIX с именем:
   /test.mq

   Тестовая среда пошлет в нее сообщение.
   Получите его и сохраните в /home/box/message.txt 
   
   Пример - http://dmilvdv.narod.ru/Translate/ELSDD/elsdd_posix_message_queue_operations.html */

#define BUF_SIZE 256

int main(int argc, char **argv)
{
    mqd_t ds;
    char buf[BUF_SIZE];
    int prio;
    const char *qName = "/test.mq";
    const char *fileName = "/home/box/message.txt";

    if ((ds = mq_open(qName, O_RDONLY, 0600, NULL)) == -1)
    {
        perror("Creating queue error");
        return 0;
    }

    if (mq_receive(ds, buf, BUF_SIZE, &prio) == -1)
    {
        perror("cannot receive");
        return -1;
    }

    printf("Message: %s, prio = %d\n", buf, prio);

    // Закрыть очередь
    if (mq_close(ds) == -1)
        perror("Closing queue error");

    // После отсоединения очередь сообщений удаляется из системы.
    if (mq_unlink(qName) == -1)
        perror("Removing queue error");

    int fd = open(fileName, O_WRONLY);
    ssize_t n_write = write(fd, buf, BUF_SIZE);

    return 0;
}
