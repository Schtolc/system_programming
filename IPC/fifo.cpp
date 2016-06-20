#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "syscall_error.h"
#include <array>


int main() {
    remove("/home/pavelgolubev345/in.fifo");
    remove("/home/pavelgolubev345/out.fifo");

    if (mkfifo("/home/pavelgolubev345/in.fifo", 0777)==-1) throw syscall_error("mkfifo");
    if (mkfifo("/home/pavelgolubev345/out.fifo",0777)==-1) throw syscall_error("mkfifo");

    auto fd_in = open("/home/pavelgolubev345/in.fifo", O_RDONLY | O_NONBLOCK);
    if (fd_in== -1) throw syscall_error("open");

    auto fd_out = open("/home/pavelgolubev345/out.fifo", O_WRONLY);
    if (fd_out== -1) throw syscall_error("open");

    auto buffer = std::array<char, 1024>();

    while (true) {
        auto nread = read(fd_in, buffer.data(), sizeof(buffer.data()));
        if (nread == -1) throw syscall_error("read");
        else if (nread > 0) {
            if (write(fd_out, buffer.data(), static_cast<size_t>(nread)) == -1) throw syscall_error("write");
        }
        sleep(1);
    }

}