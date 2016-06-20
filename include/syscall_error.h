//
// Created by pavelgolubev345 on 06.06.16.
//

#ifndef PP3_2_1_SYSCALL_ERROR_H
#define PP3_2_1_SYSCALL_ERROR_H

#include <exception>
#include <iostream>
#include <string.h>


class syscall_error : public std::exception {
public:
    syscall_error() = delete;

    syscall_error(std::string str) : functionName(str) { }

    virtual const char *what() const noexcept override {
        std::string tmp =
                "Error in syscall ***" + functionName + "*** (errno = " + std::to_string(errno) + "; " + strerror(errno) + ")";
        return tmp.c_str();
    }

private:
    std::string functionName;
};


#endif //PP3_2_1_SYSCALL_ERROR_H
