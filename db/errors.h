#pragma once
namespace aquadb
{

    enum ErrCode {
        ERR_Ok = 0,
        ERR_UNKNOWN = 100,
        ERR_NOT_FOUND = 101,
        ERR_DB_NOT_FOUND = 102,
        ERR_TBL_NOT_FOUND = 103,
        ERR_INVALID_PARAMS = 104,
        ERR_ALREADY_EXISTS = 105,
        ERR_EOF = 106,
        ERR_READ_ABNORMALITY = 107,
        ERR_WRITE_ABNORMALITY = 108,
    };
}
