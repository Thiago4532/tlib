#pragma once

#include "fdstream.hpp"
#include "glibc_wrapper.hpp"
#include <cstdlib>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

namespace tlib {

class process {
public:
    process() :
        _in(), _out(), _pid(-1), _wait_on_kill(true), _no_stdio_redirect(false), _alive(false) {  }

    explicit process(std::initializer_list<std::string> args) :
        _in(), _out(), _pid(-1), _wait_on_kill(true), _no_stdio_redirect(false), _alive(false)
    { spawn(args); }

    template<typename ...Args>
    explicit process(Args ...args) :
        _in(), _out(), _pid(-1), _wait_on_kill(true), _no_stdio_redirect(false), _alive(false), _status(-1), _signal(-1)
    { spawn(args...); }

    process(process const&) = delete;

    process(process&& __rhs) {
        _in = std::move(__rhs._in);
        _out = std::move(__rhs._out);
        _pid = __rhs._pid;
        _alive = __rhs._alive;
        _status = __rhs._status;
        _signal = __rhs._signal;

        _wait_on_kill = __rhs._wait_on_kill;
        _no_stdio_redirect = __rhs._no_stdio_redirect;
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

            if (!_no_stdio_redirect) {
                dup(fd_w[0], STDIN_FILENO);
                dup(fd_r[1], STDOUT_FILENO);
            }

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
        __wait(WNOHANG);
        return _alive;
    }

    int status() const { return _status; }
    int signal() const { return _signal; }
    const char* ssignal() const { return _signal < 0 ? NULL : strsignal(_signal); }

    void wait() { __wait(0); }

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

    // TODO: Maybe return 'process&' instead of ifdstream/ofdstream
    template<typename T>
    std::istream& operator>>(T&& __rhs) { return _in >> __rhs; }
    template<typename T>
    std::ostream& operator<<(T&& __rhs) { return _out << __rhs; }

    operator ifdstream&() { return _in; }
    operator ifdstream const&() const { return _in; }
    operator ofdstream&() { return _out; }
    operator ofdstream const&() const { return _out; }

    // A hack to make functions like 'std::endl' work.
    std::ostream& operator<<(std::ostream& (*f)(std::ostream&)) {
        f(_out);
        return _out;
    }
    std::istream& operator>>(std::istream& (*f)(std::istream&)) {
        f(_in);
        return _in;
    }

    // Flags setters
    void set_wait_on_kill(bool flag) { _wait_on_kill = flag; }
    void disable_stdio_redirection(bool flag) { _no_stdio_redirect = flag; }
private:
    ifdstream _in;
    ofdstream _out;
    pid_t _pid;
    mutable bool _alive;
    mutable int _status;
    mutable int _signal;

    // Flags
    bool _wait_on_kill;
    bool _no_stdio_redirect;

    // Only modify mutable variables (private only)
    void __wait(int options) const {
        if (!_alive) return;

        int status;
        if (waitpid(_pid, &status, options) == 0)
            return;

        _alive = false;
        if (WIFEXITED(status))
            _status = WEXITSTATUS(status);
        if (WIFSIGNALED(status))
            _signal = WTERMSIG(status);
    }
};

}
