#pragma once

#include "fdstream.hpp"
#include "glibc_wrapper.hpp"
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

namespace tlib {

class process {
public:
    process() :
        _in(), _out(), _pid(-1), _wait_on_kill(true), _alive(false) {  }

    template<typename ...Args>
    explicit process(Args ...args) :
        _in(), _out(), _pid(-1), _wait_on_kill(true), _alive(false)
    { spawn(args...); }

    process(process const&) = delete;

    process(process&& __rhs) {
        _in = std::move(__rhs._in);
        _out = std::move(__rhs._out);
        _pid = __rhs._pid;
    }

    ~process() { this->kill(); }
 
    void spawn(std::initializer_list<std::string> args) {
        int fd_r[2]; // Parent read from this
        int fd_w[2]; // Parent write to this

        pipe(fd_r);
        pipe(fd_w);

        pid_t ppid_before_fork = getpid();
        pid_t pid = fork([&]() {
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

        _alive = true;
        _pid = pid;
        _in = ifdstream(fd_r[0]);
        _out = ofdstream(fd_w[1]);
    }

    template<typename ...Args>
    void spawn(Args ...args) {
        return this->spawn(std::initializer_list<std::string>{args...});
    }

    bool is_alive() const {
        if (!_alive) return false;
        return _alive = (waitpid(_pid, NULL, WNOHANG) == 0);
    }

    void wait() { waitpid(_pid, NULL, 0); }

    void kill(int signal, bool wait) {
        if (!is_alive()) return;
        ::kill(_pid, signal);
        if (wait) this->wait();
    }

    void kill(int signal = SIGTERM) { return kill(signal, _wait_on_kill); }

    ifdstream& input() { return _in; }
    ifdstream const& input() const { return _in; }
    ofdstream& output() { return _out; }
    ofdstream const& output() const { return _out; }

    template<typename T>
    process& operator>>(T&& __rhs) { _in >> __rhs; return *this; }
    template<typename T>
    process& operator<<(T&& __rhs) { _out << __rhs; return *this; }

    operator ifdstream&() { return _in; }
    operator ifdstream const&() const { return _in; }
    operator ofdstream&() { return _out; }
    operator ofdstream const&() const { return _out; }

    // A hack to make functions like 'std::endl' work.
    process& operator<<(std::ostream& (*f)(std::ostream&)) {
        f(_out);
        return *this;
    }
    process& operator>>(std::istream& (*f)(std::istream&)) {
        f(_in);
        return *this;
    }

    // Flags setters
    void set_wait_on_kill(bool flag) { _wait_on_kill = flag; }
private:
    ifdstream _in;
    ofdstream _out;
    pid_t _pid;
   mutable bool _alive;

    // Flags
    bool _wait_on_kill;
};

}
