//
// Created by pavelgolubev345 on 19.06.16.
//

#include "Command.h"
#include <unistd.h>
#include "syscall_error.h"

void CommandStuff::Command::execute() {
    execvp(program.c_str(), makeArgVector(*this, options.size()));
}

void CommandStuff::Command::setProgram(std::string str) {
    program = str;
}

const std::string &CommandStuff::Command::getProgram() const {
    return program;
}

void CommandStuff::Command::addOption(std::string str) {
    options.push_back(str);
}

const std::string &CommandStuff::Command::optionsAt(int i) const {
    return options[i];
}

char **::CommandStuff::makeArgVector(const CommandStuff::Command &command, size_t size) {
    char **argv = new char *[size + 2];
    argv[0] = new char[command.getProgram().length() + 1];
    strcpy(argv[0], command.getProgram().c_str());

    for (auto i = 0; i < size; ++i) {
        argv[i + 1] = new char[command.optionsAt(i).length() + 1];
        strcpy(argv[i + 1], command.optionsAt(i).c_str());
    }
    argv[size+1] = NULL;
    return argv;
}
