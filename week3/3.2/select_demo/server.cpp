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
    std::set<int> SlaveSockets; // набор дескрипторов slave-сокетов
    
    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(12345);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));

    set_nonblock(MasterSocket);

    listen(MasterSocket, SOMAXCONN);

    while(true) {
        /*
         * Для select'a нужно подготовить fd_set так, чтобы были выставлены биты в позициях, соответствующих сокетам, которые нужно мониторить на возможность чтения(наш случай), записи или ошибок(3й пар-р?)
         */
        fd_set Set; // 1024 бита
        FD_ZERO(&Set);                  
        FD_SET(MasterSocket, &Set); // добавить MasterSocket в Set, чтобы иметь возможность принимать соединения
        // добавить все slave-сокеты из SlaveSockets в Set
        for(auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++) { 
            FD_SET(*Iter, &Set);
        }

        // найти дескриптор сокета (Master или Slave) с максимальным номером
        int Max = std::max(MasterSocket, *std::max_element(SlaveSockets.begin(),SlaveSockets.end()));

        // select usage
        select(Max+1,   // значение максимального дескритора среди след. трёх параметров +1
            &Set,       // ds_set дескрипторов, которые будут отслеживаться сервисом/системным вызовом select на их доступность для чтения
            NULL,       // ds_set дескрипторов, которые . . . для записи
            NULL,       // ds_set дескрипторов, The  file  descriptors  in "exceptfds"  will  be watched for exceptional conditions
            NULL);      // timeout, в течении которого будет выполняться select
        /*
         * По итогу select'a, в Set будут выставлены '1' на местах, которые соответствуют сокетам, по которым можно выполнить чтение
         */

        // Проверяем выставлен ли бит в Set в позиции, соответствующей какому-либо из имеющихся slave-сокетов (т.е. соединение уже установлено)
        for(auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++) {
            if(FD_ISSET(*Iter,&Set)) {
                // Если бит выставлен, значит пришло событие о том, что сокет доступен для чтения
                static char Buffer[1024];
                int RecvSize = recv(*Iter, Buffer, 1024, MSG_NOSIGNAL); // *note: можно читать сразу весь буффер, так как все сокеты перед добавлением в SlaveSockets делаются неблокирующими
                if((RecvSize == 0) && (errno != EAGAIN)) {
                    // Мы в ситуации, когда у нас нет реально прочитанных данных
                    //  - было прочитано 0 байт
                    //  - или "EAGAIN" - there is no data available right now, try again later
                    shutdown(*Iter, SHUT_RDWR); // разрываем предварительно установленное соединение
                    close(*Iter);               // закрываем соответствующий сокет
                    SlaveSockets.erase(Iter);   // удалям дескриптор сокета, описывающий установленное соединение
                } else if (RecvSize != 0) { // если что-то принято
                    send(*Iter, Buffer, RecvSize, MSG_NOSIGNAL);    // реализуем echo
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