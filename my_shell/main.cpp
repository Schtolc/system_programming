#include "Shell.h"

int main() {

    auto tmp = std::string();
    auto newCommand = CommandStuff::Command();
    auto shell = ShellStaff::Shell("/home/pavelgolubev345/result.out");
    auto isOption = false;

    while (std::cin >> tmp) {
        if (!isOption) {
            newCommand.setProgram(tmp);
            isOption = true;
        } else if (tmp[0] != '|') {
            newCommand.addOption(tmp);
        } else {
            shell.addCommand(newCommand);
            newCommand = CommandStuff::Command();
            isOption = false;
        }

    }
    shell.addCommand(newCommand);

    shell.run();
    return 0;
}