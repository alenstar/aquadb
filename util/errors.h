#pragma once

#include <errno.h>
#include <string.h>

namespace util {

enum Errors : int {
    ENO_SUCC = 0,
    // 1 --- 1000 保留为系统错误码
    ENO_FAIL          = 1001,
    ENO_INVALID_PARAM = 1002,
    ENO_DATA_RD_FAIL  = 1003,
    ENO_NOT_FOUND = 1004,
    ENO_UNSUPPORTED = 1005,
};

}
