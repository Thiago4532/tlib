#include <bits/stdc++.h>
#include "glibc_wrapper.hpp"
#include "fdstream.hpp"
#include "spawn.hpp"
#include <ext/stdio_filebuf.h>

using namespace std;
using namespace tlib;

int main() {
    auto [in, out] = spawn("/bin/cat");
    sleep(3);
    return 0;
}
