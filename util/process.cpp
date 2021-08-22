
#include "process.h"
#include <iostream>


namespace util {
int ProcessExec(const char* cmd, std::string& out) {
    char buffer[512] = {0x00};
    FILE* pipe = ::popen(cmd, "r");
    // if (!pipe) throw std::runtime_error("popen() failed!");
    if (!pipe) return -1;
    while (!feof(pipe)) {
        if (fgets(buffer, 512, pipe) != nullptr)
            out += std::string(buffer);
    }
    return ::pclose(pipe);
}
}
