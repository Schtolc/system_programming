//
// Created by pavelgolubev345 on 22.06.16.
//

#ifndef PP5_4_3_SENDFD_H
#define PP5_4_3_SENDFD_H

#include <iostream>

namespace SocketUtils {
    //Pass file descriptor fd through socket sock, buf cant be empty
    std::size_t sock_fd_write(int sock, void *buf, std::size_t buflen, int fd);

    //Receive file descriptor fd through socket sock, buf cant be empty
    std::size_t sock_fd_read(int sock, void *buf, std::size_t bufsize, int *fd);

}


#endif //PP5_4_3_SENDFD_H
