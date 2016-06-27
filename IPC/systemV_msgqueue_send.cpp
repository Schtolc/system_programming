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

    auto msqid = msgget(msgkey, 0);
    if (msqid == -1) throw syscall_error("msgget");

    auto msg = message{};
    msg.mtype = 2;
    msg.mtext[0] = 'q';
    msg.mtext[1] = '!';
    msg.mtext[2] = '\0';	
    if (msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1)
        throw syscall_error("msgsnd");

    if(msgctl(msqid, IPC_RMID, 0) == -1) throw syscall_error("msgctl");

    return 0;
}
