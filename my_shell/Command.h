//
// Created by pavelgolubev345 on 19.06.16.
//

#ifndef PP5_4_1_COMMAND_H
#define PP5_4_1_COMMAND_H

#include <iostream>
#include <vector>

namespace CommandStuff {
    class Command {
    private:
        std::string program;
        std::vector<std::string> options;
    public:
        Command() : program(), options() { }

        void setProgram(std::string);

        const std::string &getProgram() const;

        void addOption(std::string);

        const std::string &optionsAt(int i) const;

        void execute();
    };


    char** makeArgVector(const Command& command, size_t size);
}



#endif //PP5_4_1_COMMAND_H
