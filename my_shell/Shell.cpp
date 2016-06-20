//
// Created by pavelgolubev345 on 19.06.16.
//

#include "Shell.h"
#include <fcntl.h>
#include <unistd.h>
#include "syscall_error.h"


void ShellStaff::Shell::run() {
    //last command
    if (commands.size() == 1) {
        stdoutToFile(resultPath.c_str());
        commands[0].execute();
    } else
        processCommand(0);
}

void ShellStaff::Shell::processCommand(size_t pos) {

    int pfd[2];
    if (pipe(pfd) == -1) throw syscall_error("pipe");

    auto pid = fork();
    if (pid == -1) throw syscall_error("fork");
    if (pid == 0) {
        pipeOpenStdout(pfd);
        commands[pos].execute();
    } else {
        pipeOpenStdin(pfd);

        //last command
        if (++pos == commands.size() - 1) {
            stdoutToFile(resultPath.c_str());
            commands[pos].execute();
        //recursively process next command
        } else {
            processCommand(pos);
        }
    }
}

void ShellStaff::Shell::addCommand(const CommandStuff::Command &command) {
    commands.push_back(command);
}

void ::ShellStaff::pipeOpenStdout(int *pfd) {
    close(STDOUT_FILENO);
    if (dup2(pfd[1], STDOUT_FILENO) == -1) throw syscall_error("dup2");
    close(pfd[1]);
    close(pfd[0]);
}

void ::ShellStaff::pipeOpenStdin(int *pfd) {
    close(STDIN_FILENO);
    if (dup2(pfd[0], STDIN_FILENO) == -1) throw syscall_error("dup2");
    close(pfd[1]);
    close(pfd[0]);
}

void ::ShellStaff::stdoutToFile(const char *path) {
    auto fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0777);
    if (fd == -1) throw syscall_error("open");
    if (dup2(fd, STDOUT_FILENO) == -1) throw syscall_error("dup2");
    close(fd);
}
