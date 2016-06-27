#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "syscall_error.h"

struct message {
    long mtype;
    char mtext[80];
};

int main() {
    auto msgkey = ftok("/tmp/msg.temp", 1);
    if (msgkey == -1) throw syscall_error("ftok");

    auto msqid = msgget(msgkey, 0777 | IPC_CREAT);
    if (msqid == -1) throw syscall_error("msgget");

    auto msg = message{};
    if (msgrcv(msqid, &msg, sizeof(msg.mtext), 0, 0) == -1)
        throw syscall_error("msgrcv");

    auto fd = open("/home/box/message.txt", O_WRONLY | O_CREAT, 0777);
    if (fd == -1) throw syscall_error("open");

    auto nwrite = write(fd, &msg.mtext, sizeof(msg.mtext));
    if (nwrite == -1) throw syscall_error("write");

    close(fd);
    if(msgctl(msqid, IPC_RMID, 0) == -1) throw syscall_error("msgctl");

    return 0;
}
