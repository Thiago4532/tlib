#pragma once

#include "fdstream.hpp"
#include "glibc_wrapper.hpp"
#include <sys/prctl.h>
#include <signal.h>

namespace tlib {

inline std::pair<ifdstream, ofdstream>
spawn(std::initializer_list<std::string> args) {
    int fd_r[2]; // Parent read from this
    int fd_w[2]; // Parent write to this

    pipe(fd_r);
    pipe(fd_w);

    pid_t ppid_before_fork = getpid();
    fork([&]() {
        HANDLE_ERRNO(prctl(PR_SET_PDEATHSIG, SIGTERM));
        if (getppid() != ppid_before_fork)
            _exit(1);

        dup(fd_w[0], STDIN_FILENO);
        dup(fd_r[1], STDOUT_FILENO);

        close(fd_w[1]);
        close(fd_r[0]);

        execv(args);
    });

    close(fd_w[0]);
    close(fd_r[1]);

    return {ifdstream(fd_r[0]), ofdstream(fd_w[1])};
}

template <typename ...Args>
inline std::pair<ifdstream, ofdstream>
spawn(Args... args) {
    return spawn(std::initializer_list<std::string>{args...});
}

}
