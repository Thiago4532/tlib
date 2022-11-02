#pragma once

#include <vector>
#include <string>
#include <unistd.h>
#include <system_error>
#include "fdstream.hpp"

// Make it easier to use normal glibc functions
#define HANDLE_ERRNO(expr) ({ \
    if (expr == -1) \
        throw std::system_error(errno, std::generic_category(), #expr); \
    expr; \
})

namespace tlib {

// C++ wrappers to glibc functions
inline namespace glibc {

inline void execv(std::initializer_list<std::string> args) {
    char** argv = new char*[args.size() + 1];

    int i = 0;
    for (auto it = args.begin(); it != args.end(); it++)
        argv[i++] = const_cast<char*>(it->c_str());
    argv[i] = nullptr;

    ::execv(argv[0], argv);
    throw std::system_error(errno, std::generic_category(), "execv");
}

template <typename ...Args>
inline void execv(Args... args) {
    return execv(std::initializer_list<std::string>{args...});
}

inline void dup(int oldfd, int newfd) {
    if (::dup2(oldfd, newfd) == -1)
        throw std::system_error(errno, std::generic_category(), "dup2");
}

template <typename FunctionType>
inline pid_t fork(FunctionType child_function) {
    pid_t pid = ::fork();
    if (pid == -1)
        throw std::system_error(errno, std::generic_category(), "fork");

    if (pid == 0) {
        child_function();
        // WARNING: Destructors are not called for child process
        _exit(0);
    }

    return pid;
}

inline void pipe(int fd[2]) {
    if (::pipe(fd) == -1) 
        throw std::system_error(errno, std::generic_category(), "pipe");
}

inline std::pair<ifdstream, ofdstream> 
pipe(ifdstream& in, ofdstream& out) {
    int fd[2];
    if (::pipe(fd) == -1) 
        throw std::system_error(errno, std::generic_category(), "pipe");

    return {ifdstream(fd[0]), ofdstream(fd[1])};
}

}

}
