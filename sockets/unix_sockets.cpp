#include <errno.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "assert.h"
#include "sys/socket.h"

#define SOCKETNAME "SelectSocket"


static bool run_server(struct sockaddr *sa) {
    int fd_sock, fd_client, fd_max = 0;
    char buffer[100];
    fd_set set, tmp;
    ssize_t nread;

    assert((fd_sock = socket(AF_UNIX, SOCK_STREAM, 0)) != -1);
    assert(bind(fd_sock, sa, sizeof(*sa)) != -1);
    assert(listen(fd_sock, SOMAXCONN) != -1);

    if (fd_sock > fd_max)
        fd_max = fd_sock;
    FD_ZERO(&set);
    FD_SET(fd_sock, &set);

    while (true) {
        tmp = set;
        assert(select(fd_max + 1, &tmp, NULL, NULL, NULL) != -1);
        for (int fd = 0; fd <= fd_max; ++fd) {
            if (FD_ISSET(fd, &tmp)) {
                if (fd == fd_sock) {
                    assert((fd_client = accept(fd_sock, NULL, 0)) != -1);
                    FD_SET(fd_client, &set);
                    if (fd_client > fd_max)
                        fd_max = fd_client;
                }
                else {
                    assert((nread = read(fd, buffer, sizeof(buffer))) != -1);
                    if (nread == 0) {
                        FD_CLR(fd, &set);
                        if (fd == fd_max) {
                            fd_max--;
                        }
                        assert(close(fd) != -1);
                    } else {
                        printf("Client says %s\n", buffer);
                        assert(write(fd, "Hey client!", 12) != -1);
                    }
                }
            }
        }
    }
    assert(close(fd_sock) != -1);
    return true;
}


static bool run_client(struct sockaddr *sa) {

    if (fork() == 0) {
        int fd_sock;
        char buffer[200];

        assert((fd_sock = socket(AF_UNIX, SOCK_STREAM, 0)) != -1);
        while (connect(fd_sock, sa, sizeof(*sa)) == -1) {
            if (errno == ENOENT) {
                sleep(1);
                continue;
            } else {
                exit(EXIT_FAILURE);
            }
        }
        snprintf(buffer, sizeof(buffer), "Hey guys, I\'m %d", (int) getpid());
        assert(write(fd_sock, buffer, strlen(buffer) + 1) != -1);
        assert(read(fd_sock, buffer, sizeof(buffer)) != -1);
        printf("Server says %s\n", buffer);
        assert(close(fd_sock) != -1);
        exit(EXIT_SUCCESS);
    }
    return true;
}

int main() {
    struct sockaddr sa;

    unlink(SOCKETNAME);
    strcpy(sa.sa_data, SOCKETNAME);
    sa.sa_family = AF_UNIX;
    for (int client = 1; client <= 4; client++) {
        assert(run_client(&sa));
    }
    assert(run_server(&sa));

    return 0;
}