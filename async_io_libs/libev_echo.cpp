#include <netinet/in.h>
#include <stdio.h>
#include <ev.h>
#include "syscall_error.h"

static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    char buffer[1024];
    ssize_t nread = recv(watcher->fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
    if (nread < 0) throw syscall_error("recv");
    else if (nread == 0) {
        ev_io_stop(loop, watcher);
        free(watcher);
        return;
    } else {
        if (send(watcher->fd, buffer, nread, MSG_NOSIGNAL) == -1) throw syscall_error("send");

    }

}

static void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    int client_fd = accept(watcher->fd, NULL, 0);
    if (client_fd == -1) throw syscall_error("accept");

    struct ev_io *w_client = (struct ev_io *) malloc(sizeof(struct ev_io));
    ev_io_init(w_client, read_cb, client_fd, EV_READ);
    ev_io_start(loop, w_client);

}


int main() {
    struct ev_loop *loop = ev_default_loop();
    if (!loop) throw syscall_error("ev_default_loop");

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) throw syscall_error("socket");

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(12345);

    if (bind(fd, (struct sockaddr *) &sin, sizeof(sin)) == -1) throw syscall_error("bind");
    if (listen(fd, SOMAXCONN) == -1) throw syscall_error("listen");

    struct ev_io w_accept;
    ev_io_init(&w_accept, accept_cb, fd, EV_READ);
    ev_io_start(loop, &w_accept);

    while (1) {
        ev_loop(loop, 0);
    }
    return 0;


}