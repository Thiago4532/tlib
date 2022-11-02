#pragma once

#include <iostream>
#include <ext/stdio_filebuf.h>

namespace tlib {

class ifdstream : public std::istream {
public:
    typedef __gnu_cxx::stdio_filebuf<char> 	__filebuf_type;
    typedef std::istream 	                __istream_type;

private:
    __filebuf_type _M_filebuf;
    int _M_fd;

public:
    ifdstream()
        : __istream_type(), _M_filebuf(), _M_fd(-1)
    { this->init(&_M_filebuf); }

    explicit ifdstream(int fd, std::ios_base::openmode mode = std::ios_base::in)
        : __istream_type(), _M_filebuf(fd, mode), _M_fd(fd)
    { this->init(&_M_filebuf); }

    ifdstream(const ifdstream&) = delete;

    ifdstream(ifdstream&& __rhs)
        : __istream_type(std::move(__rhs)),
        _M_filebuf(std::move(__rhs._M_filebuf)),
        _M_fd(__rhs._M_fd)
    { __istream_type::set_rdbuf(&_M_filebuf); }

    ~ifdstream() { }

    ifdstream& operator=(const ifdstream&) = delete;

    ifdstream& operator=(ifdstream&& __rhs) {
        __istream_type::operator=(std::move(__rhs));
        _M_filebuf = std::move(__rhs._M_filebuf);
        _M_fd = __rhs._M_fd;
        return *this;
    }

    void swap(ifdstream& __rhs) {
        __istream_type::swap(__rhs);
        _M_filebuf.swap(__rhs._M_filebuf);
        std::swap(_M_fd, __rhs._M_fd);
    }

    __filebuf_type* rdbuf() const
    { return const_cast<__filebuf_type*>(&_M_filebuf); }
    
    bool is_open() { return _M_filebuf.is_open(); }
    bool is_open() const { return _M_filebuf.is_open(); }

    void close() {
        if (!_M_filebuf.close())
            this->setstate(ios_base::failbit);
    }

    int fd() const { return _M_fd; }
};

class ofdstream : public std::ostream {
public:
    typedef __gnu_cxx::stdio_filebuf<char> 	__filebuf_type;
    typedef std::ostream 	                __ostream_type;

private:
    __filebuf_type _M_filebuf;
    int _M_fd;

public:
    ofdstream()
        : __ostream_type(), _M_filebuf(), _M_fd(-1)
    { this->init(&_M_filebuf); }

    explicit ofdstream(int fd, std::ios_base::openmode mode = std::ios_base::out)
        : __ostream_type(), _M_filebuf(fd, mode), _M_fd(fd)
    { this->init(&_M_filebuf); }

    ofdstream(const ofdstream&) = delete;

    ofdstream(ofdstream&& __rhs)
        : __ostream_type(std::move(__rhs)),
        _M_filebuf(std::move(__rhs._M_filebuf)),
        _M_fd(__rhs._M_fd)
    { __ostream_type::set_rdbuf(&_M_filebuf); }

    ~ofdstream() { }

    ofdstream& operator=(const ofdstream&) = delete;

    ofdstream& operator=(ofdstream&& __rhs) {
        __ostream_type::operator=(std::move(__rhs));
        _M_filebuf = std::move(__rhs._M_filebuf);
        _M_fd = __rhs._M_fd;
        return *this;
    }

    void swap(ofdstream& __rhs) {
        __ostream_type::swap(__rhs);
        _M_filebuf.swap(__rhs._M_filebuf);
    }

    __filebuf_type* rdbuf() const
    { return const_cast<__filebuf_type*>(&_M_filebuf); }
    
    bool is_open() { return _M_filebuf.is_open(); }
    bool is_open() const { return _M_filebuf.is_open(); }

    void close() {
        if (!_M_filebuf.close())
            this->setstate(ios_base::failbit);
    }

    int fd() const { return _M_fd; }
};

}
