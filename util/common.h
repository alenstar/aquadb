// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#if defined _WIN32

// Memory leak checks
#define CHECK_HEAP_INTEGRITY()

// Debug definitions
#if defined( _DEBUG )
#include <crtdbg.h>
#undef CHECK_HEAP_INTEGRITY
#define CHECK_HEAP_INTEGRITY()                                                                                         \
    {                                                                                                                  \
        if ( !_CrtCheckMemory() ) PanicAlert( "memory corruption detected. see log." );                                \
    }
// If you want to see how much a pain in the ass singletons are, for example:
// {614} normal block at 0x030C5310, 188 bytes long.
// Data: <Master Log      > 4D 61 73 74 65 72 20 4C 6F 67 00 00 00 00 00 00
struct CrtDebugBreak {
    CrtDebugBreak( int spot ) { _CrtSetBreakAlloc( spot ); }
};
// CrtDebugBreak breakAt(614);
#endif // end DEBUG/FAST

#endif

// Windows compatibility
#ifndef _WIN32
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

#ifdef _MSC_VER
#define __getcwd _getcwd
#define __chdir _chdir
#else
#define __getcwd getcwd
#define __chdir chdir
#endif

// Dummy macro for marking translatable strings that can not be immediately translated.
// wxWidgets does not have a true dummy macro for this.
#define _trans( a ) a

#ifndef UNUSED
#define UNUSED( x ) ( (void) ( x ) )
#endif

#if !defined( COMMON_EXPORT )
#if defined( WIN32 ) || defined( _WIN32 )
#define COMMON_EXPORT __declspec( dllexport )
#else
#define COMMON_EXPORT __attribute__( ( visibility( "default" ) ) )
#endif
#endif

#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <cmath>
#include <cstring>
#include <cassert>
#include <cstdio>

//#include <cerrno>
#include <thread>
#include <stdexcept>
#include <chrono>
#include <ostream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include <string>
#include <memory>
#include <typeindex>
#include <type_traits>


#include <limits>
#include <iomanip>
#include <algorithm>

/*
#include <absl/strings/string_view.h>
#include <absl/strings/charconv.h>
#include <absl/strings/str_format.h>
#include <absl/strings/numbers.h>
#include <absl/types/any.h>
#include <absl/time/time.h>
#include <absl/time/civil_time.h>
*/

namespace util {
// using std::string;
/////////////////////////////////////////////////
/**
 * @file tc_common.h
 * @brief  帮助类,都是静态函数,提供一些常用的函数 .
 *
 */
/////////////////////////////////////////////////

class TimeCounter {
  public:
    TimeCounter() { _t = std::chrono::steady_clock::now(); }
    TimeCounter( const TimeCounter & ) = delete;
    TimeCounter &operator=( const TimeCounter & ) = delete;
    // 毫秒
    double elapse() {
        auto t = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>( t - _t ).count();
    }
    void reset() { _t = std::chrono::steady_clock::now(); }

  private:
    std::chrono::steady_clock::time_point _t;
};

inline void msleep( long long msec )
{
    std::this_thread::sleep_for(std::chrono::milliseconds(msec));
}

/*
inline int ToDateInt(absl::Time t, bool utc=false)
{
   auto cd = absl::ToCivilDay(t, utc?absl::UTCTimeZone(): absl::LocalTimeZone());
   return cd.year() *10000 + cd.month() *100 + cd.day();
}
inline int ToDateInt(absl::CivilSecond cd)
{
   return cd.year() *10000 + cd.month() *100 + cd.day();
}
inline absl::Time FromDateInt(int dt, bool utc=false)
{
    int y = dt/10000;
    int m = (dt/100 - y *100);
    int d = dt - y *10000 - m*100;
    return absl::FromCivil(absl::CivilDay(y,m, d), utc ? absl::UTCTimeZone(): absl::LocalTimeZone());
}
*/

template<typename T>
bool almost_eq(T x, T y, double epsilon = 0.000001)
{
    if (std::is_same<float, T>::value || std::is_same<double, T>::value) {
    return std::fabs(x-y) <= (epsilon * 0.5);
    }
    else {
        return x == y;
    }
}

template<typename T>
bool almost_ne(T x, T y, double epsilon = 0.000001)
{
    if (std::is_same<float, T>::value || std::is_same<double, T>::value) {
    return !almost_eq(x,y,epsilon);
    }
    else {
        return x != y;
    }
}

template<typename T>
bool almost_lt(T x, T y, double epsilon = 0.000001)
{
    if (std::is_same<float, T>::value || std::is_same<double, T>::value) {
    return  (x < y) && (almost_ne(x,y,epsilon));
    }
    else {
        return x < y;
    }
}
template<typename T>
bool almost_le(T x, T y, double epsilon = 0.000001)
{
    if (std::is_same<float, T>::value || std::is_same<double, T>::value) {
    return  (x <= y) || almost_eq(x, y, epsilon);
    }
    else {
        return x <= y;
    }
}

template<typename T>
bool almost_gt(T x, T y, double epsilon = 0.000001)
{
    if (std::is_same<float, T>::value || std::is_same<double, T>::value) {
    return  (x > y) && (almost_ne(x,y,epsilon));
    }
    else {
        return x > y;
    }
}
template<typename T>
bool almost_ge(T x, T y, double epsilon = 0.000001)
{
    if (std::is_same<float, T>::value || std::is_same<double, T>::value) {
    return  (x >= y) || almost_eq(x, y, epsilon);
    }
    else {
        return x >= y;
    }
}

/*
class StringSplit
{
public:
    StringSplit(const absl::string_view& str)
    {
        _sv = str;
    }
    StringSplit(const std::string& str)
    {
        _sv = str;
    }

    bool HasNext()
    {
        return !_sv.empty();
    }

    absl::string_view NextAsStringView(char sep = ' ', bool skipEmpty = true){
        absl::string_view sv;
        std::string str;
        for (;;){
        auto pos = _sv.find_first_of(sep);
        if(pos == std::string::npos)
        {
            _sv.swap(sv);
            return sv;
        }
        sv = _sv.substr(0, pos);
        _sv = _sv.substr(pos);
        if(sv.empty() && skipEmpty && (!_sv.empty()))
        {
            continue;
        }
        break;
        }
        return sv;
    }

    absl::string_view NextAsStringView(const std::string& sep, bool skipEmpty = true){
        absl::string_view sv;
        std::string str;
        for (;;){
        auto pos = _sv.find(sep);
        if(pos == std::string::npos)
        {
            _sv.swap(sv);
            return sv;
        }
        sv = _sv.substr(0, pos);
        _sv = _sv.substr(pos);
        if(sv.empty() && skipEmpty && (!_sv.empty()))
        {
            continue;
        }
        break;
        }
        return sv;
    }

    std::string NextAsString(char sep = ' ', bool skipEmpty=true){
        auto sv = NextAsStringView(sep, skipEmpty);
        return std::string(sv.data(), sv.size());
    }

    int NextAsInt(char sep = ' ', bool skipEmpty=true){
        int v = 0;
        absl::SimpleAtoi(NextAsStringView(sep, skipEmpty), &v);
        return v;
    }

    int64_t NextAsLong(char sep = ' ', bool skipEmpty=true){
        int64_t v = 0;
        absl::SimpleAtoi(NextAsStringView(sep, skipEmpty), &v);
        return v;
    }
    double NextAsDouble(char sep = ' ', bool skipEmpty=true){
        double v = 0;
        absl::SimpleAtod(NextAsStringView(sep, skipEmpty), &v);
        return v;
    }
    bool NextAsBool(char sep = ' ', bool skipEmpty=true){
        bool v = false;
        absl::SimpleAtob(NextAsStringView(sep, skipEmpty), &v);
        return v;
    }

private:
    absl::string_view _sv;
};
*/


template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}


/**
 * @brief  基础工具类，提供了一些非常基本的函数使用.
 *
 * 这些函数都是以静态函数提供。 包括以下几种函数:
 *
 * Trim类函数,大小写转换函数,分隔字符串函数（直接分隔字符串，
 *
 * 数字等）,时间相关函数,字符串转换函数,二进制字符串互转函数,
 *
 * 替换字符串函数,Ip匹配函数,判断一个数是否是素数等
 */
//namespace common {
/**
 * @brief  去掉头部以及尾部的字符或字符串.
 *
 * @param sStr    输入字符串
 * @param s       需要去掉的字符
 * @param bChar   如果为true, 则去掉s中每个字符; 如果为false, 则去掉s字符串
 * @return std::string 返回去掉后的字符串
 */
std::string trim( const std::string &sStr, const std::string &s = " \r\n\t", bool bChar = true );

/**
 * @brief  去掉左边的字符或字符串.
 *
 * @param sStr    输入字符串
 * @param s       需要去掉的字符
 * @param bChar   如果为true, 则去掉s中每个字符; 如果为false, 则去掉s字符串
 * @return std::string 返回去掉后的字符串
 */
std::string trimleft( const std::string &sStr, const std::string &s = " \r\n\t", bool bChar = true );

/**
 * @brief  去掉右边的字符或字符串.
 *
 * @param sStr    输入字符串
 * @param s       需要去掉的字符
 * @param bChar   如果为true, 则去掉s中每个字符; 如果为false, 则去掉s字符串
 * @return std::string 返回去掉后的字符串
 */
std::string trimright( const std::string &sStr, const std::string &s = " \r\n\t", bool bChar = true );

/**
 * @brief  字符串转换成小写.
 *
 * @param sString  字符串
 * @return std::string  转换后的字符串
 */
inline std::string lower( const std::string &s ) {
    std::string sString = s;
    for ( std::string::iterator iter = sString.begin(); iter != sString.end(); ++iter ) {
        *iter = std::tolower( *iter );
    }

    return sString;
}


/**
 * @brief  字符串转换成大写.
 *
 * @param sString  字符串
 * @return std::string  转换后的大写的字符串
 */
inline std::string upper( const std::string &s ) {
    std::string sString = s;

    for ( std::string::iterator iter = sString.begin(); iter != sString.end(); ++iter ) {
        *iter = std::toupper( *iter );
    }

    return sString;
}

/**
 * @brief  字符串是否都是数字的.
 *
 * @param sString  字符串
 * @return bool    是否是数字
 */
bool isdigit( const std::string &sInput );

/**
 * @brief  字符串转换成时间结构.
 *
 * @param sString  字符串时间格式
 * @param sFormat  格式
 * @param stTm     时间结构
 * @return         0: 成功, -1:失败
 */
int str2tm( const std::string &sString, const std::string &sFormat, struct tm &stTm );

/**
 * @brief GMT格式的时间转化为时间结构
 *
 * eg.Sat, 06 Feb 2010 09:29:29 GMT, %a, %d %b %Y %H:%M:%S GMT
 *
 * 可以用mktime换成time_t, 但是注意时区 可以用mktime(&stTm)
 *
 * - timezone换成本地的秒(time(NULL)值相同) timezone是系统的 ,
 *
 * 需要extern long timezone;
 *
 * @param sString  GMT格式的时间
 * @param stTm     转换后的时间结构
 * @return         0: 成功, -1:失败
 */
int strgmt2tm( const std::string &sString, struct tm &stTm );

/**
 * @brief  时间转换成字符串.
 *
 * @param stTm     时间结构
 * @param sFormat  需要转换的目标格式，默认为紧凑格式
 * @return std::string  转换后的时间字符串
 */
std::string tm2str( const struct tm &stTm, const std::string &sFormat = "%Y%m%d%H%M%S" );

/**
 * @brief  时间转换成字符串.
 *
 * @param t        时间结构
 * @param sFormat  需要转换的目标格式，默认为紧凑格式
 * @return std::string  转换后的时间字符串
 */
std::string tm2str( const time_t &t, const std::string &sFormat = "%Y%m%d%H%M%S" );

/**
 * @brief  当前时间转换成紧凑格式字符串
 * @param sFormat 格式，默认为紧凑格式
 * @return std::string 转换后的时间字符串
 */
std::string now2str( const std::string &sFormat = "%Y%m%d%H%M%S" );

/**
 * @brief  时间转换成GMT字符串，GMT格式:Fri, 12 Jan 2001 18:18:18 GMT
 * @param stTm    时间结构
 * @return std::string GMT格式的时间字符串
 */
std::string tm2gmtstr( const struct tm &stTm );

/**
 * @brief  时间转换成GMT字符串，GMT格式:Fri, 12 Jan 2001 18:18:18 GMT
 * @param stTm    时间结构
 * @return std::string GMT格式的时间字符串
 */
std::string tm2gmtstr( const time_t &t );

/**
 * @brief  当前时间转换成GMT字符串，GMT格式:Fri, 12 Jan 2001 18:18:18 GMT
 * @return std::string GMT格式的时间字符串
 */
std::string now2gmtstr();

/**
 * @brief  当前的日期(年月日)转换成字符串(%Y%m%d).
 *
 * @return std::string (%Y%m%d)格式的时间字符串
 */
std::string nowdate2str();

/**
 * @brief  当前的时间(时分秒)转换成字符串(%H%M%S).
 *
 * @return std::string (%H%M%S)格式的时间字符串
 */
std::string nowtime2str();

/**
 * @brief  获取当前时间的的毫秒数.
 *
 * @return int64_t 当前时间的的毫秒数
 */
int64_t now2ms();

/**
 * @brief  取出当前时间的微秒.
 *
 * @return int64_t 取出当前时间的微秒
 */
int64_t now2us();

/**
 * @brief  字符串转化成T型，如果T是数值类型, 如果str为空,则T为0.
 *
 * @param sStr  要转换的字符串
 * @return T    T型类型
 */
template <typename T> T strto( const std::string &sStr );

/**
 * @brief  字符串转化成T型.
 *
 * @param sStr      要转换的字符串
 * @param sDefault  缺省值
 * @return T        转换后的T类型
 */
template <typename T> T strto( const std::string &sStr, const std::string &sDefault );

typedef bool ( *depthJudge )( const std::string &str1, const std::string &str2 );
/**
 * @brief  解析字符串,用分隔符号分隔,保存在vector里
 *
 * 例子: |a|b||c|
 *
 * 如果withEmpty=true时, 采用|分隔为:"","a", "b", "", "c", ""
 *
 * 如果withEmpty=false时, 采用|分隔为:"a", "b", "c"
 *
 * 如果T类型为int等数值类型, 则分隔的字符串为"", 则强制转化为0
 *
 * @param sStr      输入字符串
 * @param sSep      分隔字符串(每个字符都算为分隔符)
 * @param withEmpty true代表空的也算一个元素, false时空的过滤
 * @param depthJudge 对分割后的字符再次进行判断
 * @return          解析后的字符vector
 */
template <typename T>
std::vector<T> sepstr( const std::string &sStr, const std::string &sSep, bool withEmpty = false,
                       depthJudge judge = nullptr );

/**
 * @brief T型转换成字符串，只要T能够使用ostream对象用<<重载,即可以被该函数支持
 * @param t 要转换的数据
 * @return  转换后的字符串
 */
template <typename T> std::string to_string( const T &t );

/**
 * @brief  vector转换成std::string.
 *
 * @param t 要转换的vector型的数据
 * @return  转换后的字符串
 */
template <typename T> std::string to_string( const std::vector<T> &t );

/**
 * @brief  把map输出为字符串.
 *
 * @param map<K, V, D, A>  要转换的map对象
 * @return                    std::string 输出的字符串
 */
template <typename K, typename V, typename D, typename A> std::string to_string( const std::map<K, V, D, A> &t );

/**
 * @brief  map输出为字符串.
 *
 * @param multimap<K, V, D, A>  map对象
 * @return                      输出的字符串
 */
template <typename K, typename V, typename D, typename A> std::string to_string( const std::multimap<K, V, D, A> &t );

/**
 * @brief  pair 转化为字符串，保证map等关系容器可以直接用to_string来输出
 * @param pair<F, S> pair对象
 * @return           输出的字符串
 */
template <typename F, typename S> std::string to_string( const std::pair<F, S> &itPair );

/**
 * @brief  container 转换成字符串.
 *
 * @param iFirst  迭代器
 * @param iLast   迭代器
 * @param sSep    两个元素之间的分隔符
 * @return        转换后的字符串
 */
template <typename InputIter> std::string to_string( InputIter iFirst, InputIter iLast, const std::string &sSep = "|" );

/**
 * @brief  二进制数据转换成字符串.
 *
 * @param buf     二进制buffer
 * @param len     buffer长度
 * @param sSep    分隔符
 * @param lines   多少个字节换一行, 默认0表示不换行
 * @return        转换后的字符串
 */
std::string bin2str( const void *buf, size_t len, const std::string &sSep = "", size_t lines = 0 );

/**
 * @brief  二进制数据转换成字符串.
 *
 * @param sBinData  二进制数据
 * @param sSep     分隔符
 * @param lines    多少个字节换一行, 默认0表示不换行
 * @return         转换后的字符串
 */
std::string bin2str( const std::string &sBinData, const std::string &sSep = "", size_t lines = 0 );

/**
 * @brief   字符串转换成二进制.
 *
 * @param psAsciiData 字符串
 * @param sBinData    二进制数据
 * @param iBinSize    需要转换的字符串长度
 * @return            转换长度，小于等于0则表示失败
 */
int str2bin( const char *psAsciiData, unsigned char *sBinData, int iBinSize );

/**
 * @brief  字符串转换成二进制.
 *
 * @param sBinData  要转换的字符串
 * @param sSep      分隔符
 * @param lines     多少个字节换一行, 默认0表示不换行
 * @return          转换后的二进制数据
 */
std::string str2bin( const std::string &sBinData, const std::string &sSep = "", size_t lines = 0 );

/**
 * @brief  替换字符串.
 *
 * @param sString  输入字符串
 * @param sSrc     原字符串
 * @param sDest    目的字符串
 * @return std::string  替换后的字符串
 */
std::string replace( const std::string &sString, const std::string &sSrc, const std::string &sDest );

/**
 * @brief  批量替换字符串.
 *
 * @param sString  输入字符串
 * @param mSrcDest  map<原字符串,目的字符串>
 * @return std::string  替换后的字符串
 */
std::string replace( const std::string &sString, const std::map<std::string, std::string> &mSrcDest );

/**
 * @brief 匹配以.分隔的字符串，pat中*则代表通配符，代表非空的任何字符串
 * s为空, 返回false ，pat为空, 返回true
 * @param s    普通字符串
 * @param pat  带*则被匹配的字符串，用来匹配ip地址
 * @return     是否匹配成功
 */
bool matchperiod( const std::string &s, const std::string &pat );

/**
 * @brief  匹配以.分隔的字符串.
 *
 * @param s   普通字符串
 * @param pat vector中的每个元素都是带*则被匹配的字符串，用来匹配ip地址
 * @return    是否匹配成功
 */
bool matchperiod( const std::string &s, const std::vector<std::string> &pat );

/**
 * @brief  判断一个数是否为素数.
 *
 * @param n  需要被判断的数据
 * @return   true代表是素数，false表示非素数
 */
bool isprimenumber( size_t n );

/**
 * @brief  daemon
 */
void daemon();

/**
 * @brief  忽略管道异常
 */
void ignorepipe();

/**
 * @brief  将一个std::string类型转成一个字节 .
 *
 * @param sWhat 要转换的字符串
 * @return char    转换后的字节
 */
char x2c( const std::string &sWhat );

/**
 * @brief  大小字符串换成字节数，支持K, M, G两种 例如: 1K, 3M, 4G, 4.5M, 2.3G
 * @param s            要转换的字符串
 * @param iDefaultSize 格式错误时, 缺省的大小
 * @return             字节数
 */
size_t tosize( const std::string &s, size_t iDefaultSize );

/**
 * @brief  生成基于16进制字符的随机串
 * @param p            存储随机字符串
 * @param len          字符串大小
 */
void getrandomhexchars( char *p, unsigned int len );

bool startswith( std::string const &value, std::string const &starting );

bool endswith( std::string const &value, std::string const &ending );

bool equalsignorecase( const std::string &str1, const std::string &str2 );

bool istartswith( std::string const &value, std::string const &starting );

bool        iendswith( std::string const &value, std::string const &ending );
std::string stripspaces( const std::string &str );
std::string stripquotes( const std::string &s );


//////////////////////////////////////////////////////////

inline bool startswith(const std::string &value, const std::string &starting)
{
    if (starting.size() > value.size()) return false;
    return std::equal(starting.begin(), starting.end(), value.begin());
}

inline bool endswith(const std::string &value, const std::string &ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool equalsignorecase(const std::string &str1, const std::string &str2)
{
    return lower(str1) == lower(str2);
}

inline bool istartswith(const std::string &value, const std::string &starting)
{
    if (starting.size() > value.size()) return false;
    std::string temp = value.substr(0, starting.size());
    return equalsignorecase(lower(starting), lower(temp));
}

inline bool iendswith(const std::string &value, const std::string &ending)
{
    if (ending.size() > value.size()) return false;
    std::string temp =
        value.substr(value.size() - ending.size(), ending.size());
    return equalsignorecase(lower(ending), lower(temp));
}


//////////////////////////////////////////////////////////

namespace p {
template <typename D> struct strto1 {
    D operator()( const std::string &sStr ) {
        std::string s = "0";

        if ( !sStr.empty() ) {
            s = sStr;
        }

        std::istringstream sBuffer( s );

        D t;
        sBuffer >> t;

        return t;
    }
};

template <> struct strto1<char> {
    char operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return sStr.at(0);
        }
        return 0;
    }
};

template <> struct strto1<short> {
    short operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return atoi( sStr.c_str() );
        }
        return 0;
    }
};

template <> struct strto1<unsigned short> {
    unsigned short operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return strtoul( sStr.c_str(), NULL, 10 );
        }
        return 0;
    }
};

template <> struct strto1<int> {
    int operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return atoi( sStr.c_str() );
        }
        return 0;
    }
};

template <> struct strto1<unsigned int> {
    unsigned int operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return strtoul( sStr.c_str(), NULL, 10 );
        }
        return 0;
    }
};

template <> struct strto1<long> {
    long operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return atol( sStr.c_str() );
        }
        return 0;
    }
};

template <> struct strto1<long long> {
    long long operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return atoll( sStr.c_str() );
        }
        return 0;
    }
};

template <> struct strto1<unsigned long> {
    unsigned long operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return strtoul( sStr.c_str(), nullptr, 10 );
        }
        return 0;
    }
};

template <> struct strto1<float> {
    float operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return atof( sStr.c_str() );
        }
        return 0;
    }
};

template <> struct strto1<double> {
    double operator()( const std::string &sStr ) {
        if ( !sStr.empty() ) {
            return atof( sStr.c_str() );
        }
        return 0;
    }
};

template <typename D> struct strto2 {
    D operator()( const std::string &sStr ) {
        std::istringstream sBuffer( sStr );

        D t;
        sBuffer >> t;

        return t;
    }
};

template <> struct strto2<std::string> {
    std::string operator()( const std::string &sStr ) { return sStr; }
};

} // namespace p

template <typename T> inline T strto( const std::string &sStr ) {
    using strto_type = typename std::conditional<std::is_arithmetic<T>::value, p::strto1<T>, p::strto2<T>>::type;

    return strto_type()( sStr );
}

template <typename T> inline T strto( const std::string &sStr, const std::string &sDefault ) {
    std::string s;

    if ( !sStr.empty() ) {
        s = sStr;
    } else {
        s = sDefault;
    }

    return strto<T>( s );
}

template <> inline std::string to_string<bool>( const bool &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<char>( const char &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<unsigned char>( const unsigned char &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<short>( const short &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<unsigned short>( const unsigned short &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<int>( const int &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<unsigned int>( const unsigned int &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<long>( const long &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<long long>( const long long &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<unsigned long>( const unsigned long &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<float>( const float &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<double>( const double &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<long double>( const long double &t ) {
    return std::to_string( t );
}

template <> inline std::string to_string<std::string>( const std::string &t ) { return t; }

template <typename T>
inline std::vector<T> sepstr( const std::string &sStr, const std::string &sSep, bool withEmpty, depthJudge judge ) {
    std::vector<T> vt;

    std::string::size_type pos     = 0;
    std::string::size_type pos1    = 0;
    int                    pos_tmp = -1;

    while ( true ) {
        std::string s;
        std::string s1;
        pos1 = sStr.find_first_of( sSep, pos );
        if ( pos1 == std::string::npos ) {
            if ( pos + 1 <= sStr.length() ) {
                s  = sStr.substr( -1 != pos_tmp ? pos_tmp : pos );
                s1 = "";
            }
        } else if ( pos1 == pos && ( pos1 + 1 == sStr.length() ) ) {
            s  = "";
            s1 = "";
        } else {
            s  = sStr.substr( -1 != pos_tmp ? pos_tmp : pos, pos1 - ( -1 != pos_tmp ? pos_tmp : pos ) );
            s1 = sStr.substr( pos1 + 1 );
            if ( -1 == pos_tmp ) pos_tmp = pos;
            pos = pos1;
        }

        if ( nullptr == judge || judge( s, s1 ) ) {
            if ( withEmpty ) {
                vt.push_back( strto<T>( s ) );
            } else {
                if ( !s.empty() ) {
                    T tmp = strto<T>( s );
                    vt.push_back( tmp );
                }
            }
            pos_tmp = -1;
        }

        if ( pos1 == std::string::npos ) {
            break;
        }

        pos++;
    }

    return vt;
}
template <typename T> inline std::string to_string( const T &t ) {
    std::ostringstream sBuffer;
    sBuffer << t;
    return sBuffer.str();
}

template <typename T> inline std::string to_string( const std::vector<T> &t ) {
    //std::string s;
    std::ostringstream s;
    size_t      sz = t.size();
    for ( size_t i = 0; i < sz; i++ ) {
        s << to_string( t[ i ] );
        if ( ( i + 1 ) == sz ) {
            break;
        }
        s << ",";
    }
    return s.str();
}

template <typename T> inline std::string to_string( const std::set<T> &t ) {
    //std::string s;
    std::ostringstream s;
    size_t      sz = t.size();
    for ( size_t i = 0; i < sz; i++ ) {
        s << to_string( t[ i ] );
        if ( ( i + 1 ) == sz ) {
            break;
        }
        s << ",";
    }
    return s.str();
}

template <typename T, typename D, typename A> inline std::string to_string( const std::unordered_set<T,D,A> &t ) {
    //std::string s;
    std::ostringstream s;
    size_t      sz = t.size();
    for ( size_t i = 0; i < sz; i++ ) {
        s << to_string( t.at(i) );
        if ( ( i + 1 ) == sz ) {
            break;
        }
        s << ",";
    }
    return s.str();
}

template <typename K, typename V, typename A> inline std::string to_string( const std::unordered_map<K, V, A> &t ) {
    //std::string s;
    std::ostringstream s;
    auto it = t.cbegin();
    s << "{";
    while ( it != t.cend() ) {
        s << to_string( it->first );
        s << "=";
        s << to_string( it->second );
        ++it;
        if(it != t.cend() )
        {s << ",";}
    }
    s << "}";
    return s.str();
}

template <typename K, typename V, typename D, typename A> inline std::string to_string( const std::map<K, V, D, A> &t ) {
    std::ostringstream s;
    typename std::map<K, V, D, A>::const_iterator it = t.cbegin();
    s << "{";
    while ( it != t.cend() ) {
        s << to_string( it->first );
        s << "=";
        s << to_string( it->second );
        ++it;
        if(it != t.end() )
        {s << ",";}
    }
    s << "}";
    return s.str();
}

template <typename K, typename V, typename D, typename A> inline std::string to_string( const std::multimap<K, V, D, A> &t ) {
    std::ostringstream s;
    typename std::multimap<K, V, D, A>::const_iterator it = t.begin();
    while ( it != t.end() ) {
        s << to_string( it->first );
        s << "=";
        s << to_string( it->second );
        ++it;
        if(it != t.end() )
        s << ",";
    }
    s << "}";
    return s.str();
}

template <typename F, typename S> inline std::string to_string( const std::pair<F, S> &itPair ) {
    std::ostringstream s;
    s << "(";
    s << to_string( itPair.first );
    s << ",";
    s << to_string( itPair.second );
    s << ")";
    return s.str();
}

template <typename InputIter> inline std::string to_string( InputIter iFirst, InputIter iLast, const std::string &sSep ) {
    std::ostringstream  s;
    InputIter   it = iFirst;

    while ( it != iLast ) {
        s << to_string( *it );
        ++it;

        if ( it != iLast ) {
            s << sSep;
        } else {
            break;
        }
    }

    return s.str();
}

std::string string_from_formatv( const char *format, va_list args );

std::string  string_from_format( const char *format, ... )
#if !defined _WIN32
    // On compilers that support function attributes, this gives StringFromFormat
    // the same errors and warnings that printf would give.
    __attribute__( ( __format__( printf, 1, 2 ) ) )
#endif
    ;

// Cheap!
bool chararray_from_formatv( char *out, int outsize, const char *format, va_list args );

template <size_t Count> inline void chararray_from_format( char ( &out )[ Count ], const char *format, ... ) {
    va_list args;
    va_start( args, format );
    chararray_from_formatv( out, Count, format, args );
    va_end( args );
}
std::string join_strings( const std::vector<std::string> &strings, const std::string &delimiter );

//} // namespace common
} // namespace util
