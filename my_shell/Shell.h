//
// Created by pavelgolubev345 on 19.06.16.
//

#ifndef PP5_4_1_SHELL_H
#define PP5_4_1_SHELL_H


#include "Command.h"

namespace ShellStaff {
    class Shell {
    private:
        std::vector<CommandStuff::Command> commands;
        std::string resultPath;

        void processCommand(size_t pos);

    public:
        Shell(std::string path) : commands(), resultPath(path) { }

        void addCommand(const CommandStuff::Command &command);

        void run();
    };

    void pipeOpenStdout(int *fd);

    void pipeOpenStdin(int *fd);

    void stdoutToFile(const char* path);
}






#endif //PP5_4_1_SHELL_H
