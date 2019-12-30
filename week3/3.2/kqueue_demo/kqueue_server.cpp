#include <iostream>
#include <set>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <kqueue/sys/event.h>

#define MAX_EVENTS 32

int set_nonblock(int fd) {
    	int flags;
#if defined(O_NONBLOCK)
	/* считать флаги, выставить новый флаг O_NONBLOCK, записать новые флаги */	
	if(-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}

int main(int argc, char **argv) {
    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(12345);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));

    set_nonblock(MasterSocket);

    listen(MasterSocket, SOMAXCONN);

    int KQueue = kqueue(); // дескриптор

    struct kevent KEvent;
    bzero(&KEvent, sizeof(KEvent));
    EV_SET(&KEvent, MasterSocket, EVFILT_READ, EV_ADD, 0,0,0);
    kevent(KQueue, &KEvent, 1, NULL, 0, NULL); // регистрируем 1 event - KEvent
    
    while(true) {
        printf("while - top\n");
        bzero(&KEvent, sizeof(KEvent));
        // по какой-то причине мы зависаем на вызове kevent()
        kevent(KQueue, NULL, 0, &KEvent, 1, NULL); // получаем одно событие

        printf("while - kevent returned control\n");

        if(KEvent.filter == EVFILT_READ) {
            if(KEvent.ident == MasterSocket) {
                int SlaveSocket = accept(MasterSocket,0,0);
                set_nonblock(SlaveSocket);

                bzero(&KEvent, sizeof(KEvent));
                EV_SET(&KEvent, SlaveSocket, EVFILT_READ, EV_ADD,0,0,0);
                kevent(KQueue, &KEvent,1,NULL,0,NULL);
            } else {
                static char Buffer[1024];
                int RecvSize = recv(KEvent.ident, Buffer, 1024, MSG_NOSIGNAL);
                if(RecvSize <= 0) {
                    // Внимание: если RecvSize<0, то произошла ошибка, но в случае ошибки EINTR
                    // (мы прервались по сигналу) требуется продолжить чтение (здесь этот момент опущен).
                    // Обрабатываем только случай RecvSize=0 - соединение было закрыто
                    close(KEvent.ident);
                    printf("disconnected!\n");
                } else {
                    // что-то удалось прочитать - реализуем echo
                    send(KEvent.ident, Buffer, RecvSize, MSG_NOSIGNAL);
                }
            }
        }
        printf("while - eof if logic\n");

        
    }
    
    return 0;
}