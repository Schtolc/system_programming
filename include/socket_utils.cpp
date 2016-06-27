//
// Created by pavelgolubev345 on 22.06.16.
//

#include "Sendfd.h"
#include "syscall_error.h"
#include <sys/socket.h>
#include <unistd.h>

std::size_t SocketUtils::sock_fd_write(int sock, void *buf, std::size_t buflen, int fd) {

    //Declaration
    auto msg = msghdr();
    auto iov = iovec();
    union {
        cmsghdr cmsgheader;
        char control[CMSG_SPACE(sizeof(int))];
    } cmsgu;
    struct cmsghdr *cmsg;

    //Definition
    iov.iov_base = buf;
    iov.iov_len = buflen;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    //Passing file descriptor
    if (fd != -1) {
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        printf("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    }
        //Not passing file descriptor
    else {
        msg.msg_control = nullptr;
        msg.msg_controllen = 0;

        printf("not passing fd\n");
    }

    //Sending data
    auto size = sendmsg(sock, &msg, 0);
    if (size < 0) throw syscall_error("sock_fd_write: sendmsg");
    return static_cast<size_t>(size);
}


std::size_t SocketUtils::sock_fd_read(int sock, void *buf, std::size_t bufsize, int *fd) {
    //File descriptor received
    if (fd) {
        //Declaration
        auto msg = msghdr();
        auto iov = iovec();
        union {
            cmsghdr cmsgheader;
            char control[CMSG_SPACE(sizeof(int))];
        } cmsgu;
        struct cmsghdr *cmsg;

        //Definition
        iov.iov_base = buf;
        iov.iov_len = bufsize;
        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        //receiving data
        auto size = recvmsg(sock, &msg, 0);
        if (size < 0) throw syscall_error("sock_fd_read: recvmsg");

        //Getting file descriptor
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) throw syscall_error("Invalid cmsg level");
            if (cmsg->cmsg_type != SCM_RIGHTS) throw syscall_error("Invalid cmsg rights");
            *fd = *((int *) CMSG_DATA(cmsg));
            printf("recieved fd %d\n", *fd);
        } else
            *fd = -1;

        return static_cast<size_t>(size);
    }

    //File descriptor is not received
    auto size = read(sock, buf, bufsize);
    if (size < 0) throw syscall_error("sock_fd_read: read");
    return static_cast<size_t>(size);
}
