#include <iostream>
#include <set>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

int set_nonblock(int fd) {
    	int flags;
#if defined(O_NONBLOCK)
	/* считать флаги, выставить новый флаг O_NONBLOCK, записать новые флаги*/	
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
    std::set<int> SlaveSockets; // набор дескрипторов slave-сокетов
    
    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(12345);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));

    set_nonblock(MasterSocket);

    listen(MasterSocket, SOMAXCONN);

    while(true) {
        fd_set Set; // 1024 бита
        FD_ZERO(&Set);
        FD_SET(MasterSocket, &Set); // добавить MasterSocket в Set
        // добавить все SlaveSocket в Set
        for(auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++) { 
            FD_SET(*Iter, &Set);
        }

        // найти макс дескриптор+1
        int Max = std::max(MasterSocket, *std::max_element(SlaveSockets.begin(),SlaveSockets.end()));

        // select usage
        select(Max+1, &Set, NULL, NULL, NULL);
        // в Set будут выставлены '1' на местах, которые соответствуют сокетам, по которым было выполнено чтение
        for(auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++) {
            if(FD_ISSET(*Iter,&Set)) {
                static char Buffer[1024];
                // можно читать в весь буффер, т.к. SlaveSockets неблокирующий
                int RecvSize = recv(*Iter, Buffer, 1024, MSG_NOSIGNAL);
                // Если пришло событие о том, что мы можем читать из сокета, а было прочитано 0 байт, то
                // или произошла ошибка
                // или соединение закрыто
                if((RecvSize == 0) && (errno != EAGAIN)) {
                    shutdown(*Iter, SHUT_RDWR);
                    close(*Iter);
                    SlaveSockets.erase(Iter);
                } else if (RecvSize != 0) { // если что-то принято
                    // реализуем echo
                    send(*Iter, Buffer, RecvSize, MSG_NOSIGNAL);
                }
            }
        }
        if (FD_ISSET(MasterSocket, &Set)) {
            int SlaveSocket = accept(MasterSocket,0,0);
            set_nonblock(SlaveSocket);
            SlaveSockets.insert(SlaveSocket);
        }
    }
    
    return 0;
}