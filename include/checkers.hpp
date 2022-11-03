#pragma once

#include <istream>

namespace tlib {

inline bool diff_checker(std::istream& a, std::istream& b) {
    if (a.fail() || b.fail())
        return false;

    while (!a.eof() && !b.eof()) {
        if (a.fail() || b.fail())
            return false; // TODO: Check if this is correct

        char c_a, c_b;
        a.get(c_a);
        b.get(c_b);

        if (c_a != c_b)
            return false;
    }
    return a.eof() && b.eof();
}

}
