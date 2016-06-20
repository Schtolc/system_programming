#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <algorithm>
#include <set>
#include <typeinfo>
#include <stdexcept>
#include "syscall_error.h"
#include "poll.h"
#include "array"
#include "sys/epoll.h"

void set_nonblock(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) != -1)
        flags = 0;
    if (fcntl(fd, F_SETFD, flags | O_NONBLOCK) == -1) {
        throw syscall_error("fcntl");
    }
}

void startTcpSocket(auto sock_fd, auto addr, auto port) {
    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = port;
    SockAddr.sin_addr.s_addr = addr;

    if (bind(sock_fd, (struct sockaddr *) &SockAddr, sizeof(SockAddr)) == -1)
        throw syscall_error("bind");
    set_nonblock(sock_fd);
    if (listen(sock_fd, SOMAXCONN) == -1)
        throw syscall_error("listen");
}

void selecting(auto MasterSocket) {
    std::set<decltype(MasterSocket), std::greater<decltype(MasterSocket)>> Sockets;
    Sockets.insert(MasterSocket);
    fd_set read_set, tmp;
    FD_ZERO(&read_set);
    FD_SET(MasterSocket, &read_set);

    while (true) {
        tmp = read_set;
        if (select(*Sockets.cbegin() + 1, &tmp, nullptr, nullptr, nullptr) == -1)
            throw syscall_error("select");

        for (auto sock : Sockets) {
            if (FD_ISSET(sock, &tmp)) {
                //New connection
                if (sock == MasterSocket) {
                    decltype(MasterSocket) newSocket;
                    if ((newSocket = accept(sock, nullptr, 0)) == -1)
                        throw syscall_error("accept");
                    FD_SET(newSocket, &read_set);
                    Sockets.insert(newSocket);
                } //New data
                else {
                    char buffer[50];
                    auto nread = recv(sock, buffer, sizeof(buffer), MSG_NOSIGNAL);
                    if (nread == -1) throw syscall_error("recv");
                    if (nread == 0 && errno != EAGAIN) {
                        FD_CLR(sock, &read_set);
                        shutdown(sock, SHUT_RDWR);
                        close(sock);
                        Sockets.erase(sock);
                    } else {
                        buffer[nread] = '\0';
                        if (send(sock, buffer, static_cast<size_t>(nread + 1), MSG_NOSIGNAL) == -1)
                            throw syscall_error("send");
                    }
                }
            }
        }
    }
    close(MasterSocket);
}

void polling(auto MasterSocket) {
    static const int POLL_SIZE = 2048;
    std::array<pollfd, 2048> read_set{};
    read_set[0].fd = MasterSocket;
    read_set[0].events = POLLIN;
    int read_set_size = 1;

    while (true) {
        std::cout << std::endl;
        if (poll(read_set.data(), static_cast<nfds_t >(read_set_size), -1) == -1)
            throw syscall_error("poll");
        auto tmp_size = read_set_size;
        for (auto it = read_set.cbegin(); it != read_set.cbegin() + tmp_size; it++) {
            if (it->revents & POLLIN) {
                // new connection
                if (it == read_set.cbegin()) {
                    auto SlaveSocket = accept(MasterSocket, nullptr, 0);
                    if (SlaveSocket == -1) throw syscall_error("accept");
                    set_nonblock(SlaveSocket);
                    read_set[read_set_size].fd = SlaveSocket;
                    read_set[read_set_size].events = POLLIN;
                    read_set_size++;
                } // New data
                else {
                    char buffer[50];
                    auto nread = recv(it->fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
                    if (nread == -1) throw syscall_error("recv");
                    if (nread == 0 && errno != EAGAIN) {
                        shutdown(it->fd, SHUT_RDWR);
                        close(it->fd);
                        std::remove_if(read_set.begin(), read_set.begin() + read_set_size, [&it](pollfd &item) {
                            return item.fd == it->fd;
                        });
                        read_set[--read_set_size].fd = 0;
                    } else {
                        buffer[nread] = '\0';
                        if (send(it->fd, buffer, static_cast<size_t>(nread + 1), MSG_NOSIGNAL) == -1)
                            throw syscall_error("send");
                    }
                }
            }
        }
    }
}

void epolling(auto MasterSocket) {
    static const int MAX_EVENTS = 32;
    auto EPoll = epoll_create1(0);
    if (EPoll == -1) throw syscall_error("epoll_create1");

    struct epoll_event Event;
    Event.data.fd = MasterSocket;
    Event.events = EPOLLIN;
    if(epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event) == -1)
        throw syscall_error("epoll_ctl");

    while(true) {
        struct epoll_event Events[MAX_EVENTS];
        auto N = epoll_wait(EPoll, Events, MAX_EVENTS, -1);
        if(N==-1) throw syscall_error("epoll_wait");

        for (auto i = 0; i < N; i++) {
            // New connection
            if (Events[i].data.fd == MasterSocket) {
                auto SlaveSocket = accept(MasterSocket, nullptr, 0);
                if (SlaveSocket == -1) throw syscall_error("accept");
                set_nonblock(SlaveSocket);
                struct epoll_event newEvent;
                newEvent.data.fd = SlaveSocket;
                newEvent.events = EPOLLIN;
                if (epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &newEvent) == -1)
                    throw syscall_error("epoll_ctl");
            } //New data
            else {
                char buffer[50];
                auto nread = recv(Events[i].data.fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
                if (nread == -1) throw syscall_error("recv");
                if (nread == 0 && errno != EAGAIN) {
                    shutdown(Events[i].data.fd, SHUT_RDWR);
                    close(Events[i].data.fd);
                } else {
                    buffer[nread] = '\0';
                    if (send(Events[i].data.fd, buffer, static_cast<size_t>(nread + 1), MSG_NOSIGNAL) == -1)
                        throw syscall_error("send");
                }

            }
        }
    }

}


int main(int argc, char **argv) {
    try {
        auto MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (MasterSocket == -1) {
            throw syscall_error("socket");
        }

        startTcpSocket(MasterSocket, htonl(INADDR_ANY), htons(7777));

//        selecting(MasterSocket);
//        polling(MasterSocket);
        epolling(MasterSocket);

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}