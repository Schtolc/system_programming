#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include "syscall_error.h"
#include "socket_utils.h"


int main() {
    int fd[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) throw syscall_error("socketpair");
    auto pid = fork();
    if (pid == -1) throw syscall_error("fork");

    //Child
    if (pid == 0) {
        close(fd[0]);
        auto child_fd = 0;
        auto buffer = std::array<char, 16>();
        sleep(1);
        auto nread = SocketUtils::sock_fd_read(fd[1], buffer.data(), sizeof(buffer.data()), nullptr);
        if (nread == 0)
            return 0;
        printf("read %d\n", static_cast<int>(nread));
        nread = SocketUtils::sock_fd_read(fd[1], buffer.data(), sizeof(buffer.data()), &child_fd);
        if (nread == 0)
            return 0;
        printf("read %d\n", static_cast<int>(nread));
        if (child_fd != -1) {
            write(child_fd, "I recieved fd!", 15);
            close(child_fd);
        }
    }
        //Parent
    else {
        close(fd[1]);
        auto parent_fd = 1;
        char c[] = "1";
        auto nsend = SocketUtils::sock_fd_write(fd[0], c, 1, 1);
        printf("wrote without fd%d\n", static_cast<int>(nsend));
        nsend = SocketUtils::sock_fd_write(fd[0], c, 1, 2);
        printf("wrote with fd%d\n", static_cast<int>(nsend));
    }

    return 0;

}
