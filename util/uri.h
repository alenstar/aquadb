#pragma once
#include <iostream>
#include <stdlib.h>
#include <string>

namespace util {
std::string UriEncode( const std::string &sSrc );
std::string UriDecode( const std::string &sSrc );

struct uri {
    int         port;
    std::string protocol, user, password, host, path, search;
};

//--- Public Interface -------------------------------------------------------------~
uri UriParse( const std::string &in );

} // namespace util

