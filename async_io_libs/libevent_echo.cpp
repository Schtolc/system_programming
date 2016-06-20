#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <string.h>
#include "syscall_error.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"

#define UNIX_SOCK_PATH "/tmp/echo.sock"

static void echo_read_cb(struct bufferevent *bev, void *data) {
    struct evbuffer *input = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);

    size_t length = evbuffer_get_length(input);
    char *msg = (char *) malloc(length);
    evbuffer_copyout(input, msg, length);
    printf("%s\n", msg);

    evbuffer_add_buffer(output, input);
    free(msg);
}


static void echo_event_cb(struct bufferevent *bev, short events, void *data) {
    if (events & BEV_EVENT_ERROR) {
        perror("Error in new event");
        bufferevent_free(bev);
    } else if (events & BEV_EVENT_EOF) {
        bufferevent_free(bev);
    }
}

static void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen,
                           void *data) {

    struct event_base *base = evconnlistener_get_base(listener);
    if (!base) throw syscall_error("evconnlistener_get_base");

    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) throw syscall_error("bufferevent_socket_new");

    bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
    if (bufferevent_enable(bev, EV_READ | EV_WRITE) == -1) throw syscall_error("bufferevent_enable");

}

static void accept_error_cb(struct evconnlistener *listener, void *data) {
    struct event_base *base = evconnlistener_get_base(listener);
    if (!base) throw syscall_error("evconnlistener_get_base");

    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Error = %d = \"%s\"\n", err, evutil_socket_error_to_string(err));
    if (event_base_loopexit(base, NULL) == -1) throw syscall_error("event_base_loopexit");

}


int main() {
    struct event_base *base = event_base_new();

    struct sockaddr_un sun;
    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, UNIX_SOCK_PATH);

    struct evconnlistener *listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
                                                              LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                                              (struct sockaddr *) &sun, sizeof(sun));
    if (!listener) throw syscall_error("evconnlistener_new_bind");

    evconnlistener_set_error_cb(listener, accept_error_cb);
    if (event_base_dispatch(base) == -1) throw syscall_error("event_base_dispatch");


}