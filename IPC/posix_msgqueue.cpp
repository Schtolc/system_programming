#include <sys/types.h>
#include <fcntl.h>
#include <mqueue.h>
#include <unistd.h>
#include "syscall_error.h"

int main() {

    auto attr = mq_attr{};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 4096;
    attr.mq_flags = 0;

    unlink("/test.mq");
    auto msqid = mq_open("/test.mq", O_CREAT, 0666, &attr);
    if (msqid == -1) throw syscall_error("mq_open");

    char buffer[4096];
    if(mq_receive(msqid, &buffer[0], sizeof(buffer), nullptr) == -1)
        throw syscall_error("mq_recieve");

    FILE *fd = fopen("message.txt", "w+");
    if (fd == NULL) throw syscall_error("fopen");
    fprintf(fd, "%s", buffer);
    fclose(fd);

    if (mq_close(msqid) == -1) throw syscall_error("msqid");

    return 0;
}
