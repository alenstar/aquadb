#pragma once
#include <fstream>
#include <cstdio>

namespace util {
int ProcessExec(const char* cmd, std::string& out);
}
