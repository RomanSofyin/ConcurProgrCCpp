#include <stdio.h>
#include <malloc.h> 
#include <netinet/in.h>
#include <ev.h>

// callback для запросов на чтение
void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    char buffer[1024];
    ssize_t r = recv(watcher->fd, buffer, 1024, MSG_NOSIGNAL);
    if(r < 0) {         // прочитать не удалось
        return;
    }
    else if(r == 0) {   // соединение закрылось
        ev_io_stop(loop, watcher);
        free(watcher);
        return;
    } else {            // прочитать удалось
        send(watcher->fd, buffer, r, MSG_NOSIGNAL); // реализуем echo функционал
    }
}

// callback, который будет вызван при возникшем запросе на соединение
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    int client_sd = accept(watcher->fd, 0, 0);

    // создаём новый watcher на чтение из этого соединения
    struct ev_io *w_client = (struct ev_io*) malloc(sizeof(struct ev_io));
    // регистрируем и запускаем callback для запросов на чтение
    ev_io_init(w_client, read_cb, client_sd, EV_READ);
    ev_io_start(loop, w_client);
}

int main(int argc, char **argv) {
    struct ev_loop * loop = ev_default_loop(0); // получить default loop
    
    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // получить socket descriptor

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sd, (struct sockaddr *)&addr, sizeof(addr));

    listen(sd, SOMAXCONN);

    // создать watcher для приёма соединений
    struct ev_io w_accept;
    ev_io_init(&w_accept, accept_cb, sd, EV_READ);  // инициализация вотчера
    ev_io_start(loop, &w_accept);

    while(1) ev_loop(loop, 0);

    return 0;
}








