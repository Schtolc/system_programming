#include <stdio.h>
#include <sys/wait.h>
#include "stdlib.h"
#include "unistd.h"
#include "assert.h"

static void child_waiting(int signum, siginfo_t *siginfo, void *context) {
    int status = 0;
    waitpid(siginfo->si_pid, &status, 0);
    printf("child %d terminated\n", siginfo->si_pid);
}


int main() {
    pid_t pid;
    assert((pid = fork()) != -1);
    if (pid == 0) {
        FILE *fd_child = fopen("/home/pavelgolubev345/pid_child", "w+");
        assert(fd_child != NULL);
        assert(fprintf(fd_child, "%d", getpid()) != -1);
        assert(fclose(fd_child) != -1);
        FILE *fd_parent = fopen("/home/pavelgolubev345/pid_parent", "w+");
        assert(fd_parent != NULL);
        assert(fprintf(fd_parent, "%d", getppid()) != 1);
        assert(fclose(fd_parent) != -1);
    } else {
        struct sigaction act;
        act.sa_sigaction = &child_waiting;
        sigfillset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO;

        assert(sigaction(SIGCHLD, &act, NULL) != -1);
    }
    pause();

    return 0;
}  