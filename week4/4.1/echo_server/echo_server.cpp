#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define UNIX_SOCK_PATH "/tmp/echo.sock7"

static  void echo_read_cb(struct bufferevent *bev, void *ctx) {
    struct evbuffer *input  = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);

    size_t length = evbuffer_get_length(input);
    char *data;
    data = (char*) malloc(length);
    evbuffer_copyout(input, data, length);
    printf("data: %s\n", data);

    evbuffer_add_buffer(output, input);
    free(data);
}

// если произойдёт какое-либо событие
static void echo_event_cb(struct bufferevent *bev, short events, void *ctx) {

    if(events & BEV_EVENT_ERROR) {
        perror("Error");
        bufferevent_free(bev);
    }
    if(events & BEV_EVENT_EOF) {
        bufferevent_free(bev);
    }
}

static void accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd,
    struct sockaddr *address,
    int socklen,
    void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd,
        BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener,
    void *parms = NULL)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Error = %d = \"%s\"\n", err, evutil_socket_error_to_string(err));
    // завершить event loop
    event_base_loopexit(base, NULL);
}

int main(int argc, char **argv) {
    fprintf(stderr, "very beggining\n");
    struct event_base *base = event_base_new();

    struct sockaddr_un sun; // UNIX domain socket address
    fprintf(stderr, "blabla 090\n");
    memset(&sun, 0, sizeof(sun));
    fprintf(stderr, "blabla 095\n");
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, UNIX_SOCK_PATH);
    fprintf(stderr, "blabla 100\n");
    // создание слушателя на входящие соединения
    struct evconnlistener *listener = evconnlistener_new_bind(base,
       accept_conn_cb,
       NULL,                                           /* параметр в callback */
       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
       -1,                                             /* backlog */
       (struct sockaddr *)&sun, sizeof(sun));
    
    if (!listener) {
        perror("Couldn't create listener");
        return 1;
    }

    fprintf(stderr, "blabla 150\n");

    evconnlistener_set_error_cb(listener, accept_error_cb);
    
    event_base_dispatch(base);

    return 0;
}








