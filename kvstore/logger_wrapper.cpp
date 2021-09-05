#include "logger_wrapper.h"
#include "util/logdef.h"

#undef SPDLOG_TAG
#define SPDLOG_TAG "[RAFT]"

#ifdef _WIN32
#define SHORT_FILENAME(fn) ( strrchr( fn, '\\' ) ? strrchr( fn, '\\' ) + 1 : fn)
#else
#define SHORT_FILENAME(fn) ( strrchr( fn, '/' ) ? strrchr( fn, '/' ) + 1 : fn)
#endif


    void logger_wrapper::put_details(int level,
                     const char* source_file,
                     const char* func_name,
                     size_t line_number,
                     const std::string& msg) 
    {
     /*****
     * Log level info:
     *    Trace:    6
     *    Debug:    5
     *    Info:     4
     *    Warning:  3
     *    Error:    2
     *    Fatal:    1
     * 
     *******/
    auto nowstr = my_nowtime2str();
    switch (level)
    {
    case 1:
    case 2:
        fprintf( stderr, "%s %s [E] %s::%s() %lu|%s\n", nowstr.c_str(), SPDLOG_TAG, SHORT_FILENAME(source_file), func_name, line_number, msg.c_str());   
        break;
    case 3:
        fprintf( stderr, "%s %s [W] %s::%s() %lu|%s\n",nowstr.c_str(),  SPDLOG_TAG, SHORT_FILENAME(source_file), func_name, line_number, msg.c_str());   
        break;
    case 4:
        fprintf( stderr, "%s %s [I] %s::%s() %lu|%s\n",nowstr.c_str(),  SPDLOG_TAG, SHORT_FILENAME(source_file), func_name, line_number, msg.c_str());   
        break;
    case 5:
    case 6:
    default:
        fprintf( stderr, "%s %s [D] %s::%s() %lu|%s\n",nowstr.c_str(),  SPDLOG_TAG, SHORT_FILENAME(source_file), func_name, line_number, msg.c_str());   
        break;
    }
    }

#undef SPDLOG_TAG
