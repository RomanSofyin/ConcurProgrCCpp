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

#define UNIX_SOCK_PATH "/tmp/echo.sock"

static void accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd,
    struct sockaddr *address,
    int socklen,
    void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd,
        BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, echo_read_cb, NULL, echo_write_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener,
    evutil_socket_t fd,
    struct sockaddr *address,
    int socklen,
    void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Error = %d = \"%s\"\n", err, evutil_socket_error_to_string(err));
    event_base_loopexit(base,NULL);
}

int main(int argc, char **argv) {
    struct event_base *base = event_base_new();

    struct sockaddr_un sun; // UNIX domain socket address
    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, UNIX_SOCK_PATH);

    struct evconnlistener *listener = evconnlistener_new_bind(base,
        accept_conn_cb,
        NULL,
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSABLE,
        -1, /* backlog */
        (struct sockaddr *)&sun, sizeof(sun));

    evconnlistener_set_error_cb(listener, accept_error_cb);
    
    event_base_dispatch(base);

    return 0;
}








