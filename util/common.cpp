/**
 * Tencent is pleased to support the open source community by making Tars
 * available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the
 * License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include "common.h"


namespace util {
//namespace common {

std::string trim(const std::string &sStr, const std::string &s, bool bChar)
{
    if (sStr.empty()) {
        return sStr;
    }

    /**
     * 将完全与s相同的字符串去掉
     */
    if (!bChar) {
        return trimright(trimleft(sStr, s, false), s, false);
    }

    return trimright(trimleft(sStr, s, true), s, true);
}

std::string trimleft(const std::string &sStr, const std::string &s, bool bChar)
{
    if (sStr.empty()) {
        return sStr;
    }

    /**
     * 去掉sStr左边的字符串s
     */
    if (!bChar) {
        if (sStr.length() < s.length()) {
            return sStr;
        }

        if (sStr.compare(0, s.length(), s) == 0) {
            return sStr.substr(s.length());
        }

        return sStr;
    }

    /**
     * 去掉sStr左边的 字符串s中的字符
     */
    std::string::size_type pos = 0;
    while (pos < sStr.length()) {
        if (s.find_first_of(sStr[pos]) == std::string::npos) {
            break;
        }

        pos++;
    }

    if (pos == 0) return sStr;

    return sStr.substr(pos);
}

std::string trimright(const std::string &sStr, const std::string &s, bool bChar)
{
    if (sStr.empty()) {
        return sStr;
    }

    /**
     * 去掉sStr右边的字符串s
     */
    if (!bChar) {
        if (sStr.length() < s.length()) {
            return sStr;
        }

        if (sStr.compare(sStr.length() - s.length(), s.length(), s) == 0) {
            return sStr.substr(0, sStr.length() - s.length());
        }

        return sStr;
    }

    /**
     * 去掉sStr右边的 字符串s中的字符
     */
    std::string::size_type pos = sStr.length();
    while (pos != 0) {
        if (s.find_first_of(sStr[pos - 1]) == std::string::npos) {
            break;
        }

        pos--;
    }

    if (pos == sStr.length()) return sStr;

    return sStr.substr(0, pos);
}

bool isdigit(const std::string &sInput)
{
    std::string::const_iterator iter = sInput.begin();

    if (sInput.empty()) {
        return false;
    }

    while (iter != sInput.end()) {
        if (!::isdigit(*iter)) {
            return false;
        }
        ++iter;
    }
    return true;
}

int str2tm(const std::string &sString, const std::string &sFormat,
           struct tm &stTm)
{
    char *p = strptime(sString.c_str(), sFormat.c_str(), &stTm);
    return (p != nullptr) ? 0 : -1;
}

int strgmt2tm(const std::string &sString, struct tm &stTm)
{
    return str2tm(sString, "%a, %d %b %Y %H:%M:%S GMT", stTm);
}

std::string tm2str(const struct tm &stTm, const std::string &sFormat)
{
    char sTimeString[255] = "\0";

    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), &stTm);

    return std::string(sTimeString);
}

std::string tm2str(const time_t &t, const std::string &sFormat)
{
    struct tm tt;
    localtime_r(&t, &tt);

    return tm2str(tt, sFormat);
}

std::string now2str(const std::string &sFormat)
{
    time_t t = time(nullptr);
    return tm2str(t, sFormat.c_str());
}

std::string now2gmtstr()
{
    time_t t = time(nullptr);
    return tm2gmtstr(t);
}

std::string tm2gmtstr(const time_t &t)
{
    struct tm tt;
    gmtime_r(&t, &tt);
    return tm2str(tt, "%a, %d %b %Y %H:%M:%S GMT");
}

std::string tm2gmtstr(const struct tm &stTm)
{
    return tm2str(stTm, "%a, %d %b %Y %H:%M:%S GMT");
}

std::string nowdate2str() { return now2str("%Y%m%d"); }

std::string nowtime2str() { return now2str("%H%M%S"); }

int64_t now2ms()
{
    struct timeval tv;

    gettimeofday(&tv, nullptr);

    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

int64_t now2us()
{
    struct timeval tv;

    gettimeofday(&tv, nullptr);

    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

//参照phorix的优化
static char c_b2s[256][3] = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0a", "0b",
    "0c", "0d", "0e", "0f", "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "1a", "1b", "1c", "1d", "1e", "1f", "20", "21", "22", "23",
    "24", "25", "26", "27", "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3a", "3b",
    "3c", "3d", "3e", "3f", "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "4a", "4b", "4c", "4d", "4e", "4f", "50", "51", "52", "53",
    "54", "55", "56", "57", "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6a", "6b",
    "6c", "6d", "6e", "6f", "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "7a", "7b", "7c", "7d", "7e", "7f", "80", "81", "82", "83",
    "84", "85", "86", "87", "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9a", "9b",
    "9c", "9d", "9e", "9f", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af", "b0", "b1", "b2", "b3",
    "b4", "b5", "b6", "b7", "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "ca", "cb",
    "cc", "cd", "ce", "cf", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    "d8", "d9", "da", "db", "dc", "dd", "de", "df", "e0", "e1", "e2", "e3",
    "e4", "e5", "e6", "e7", "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "fa", "fb",
    "fc", "fd", "fe", "ff"};

std::string bin2str(const void *buf, size_t len, const std::string &sSep,
                    size_t lines)
{
    if (buf == nullptr || len <= 0) {
        return "";
    }

    std::string sOut;
    const unsigned char *p = (const unsigned char *)buf;

    for (size_t i = 0; i < len; ++i, ++p) {
        sOut += c_b2s[*p][0];
        sOut += c_b2s[*p][1];
        sOut += sSep;

        //换行
        if ((lines != 0) && ((i + 1) % lines == 0)) {
            sOut += "\n";
        }
    }

    return sOut;
}

std::string bin2str(const std::string &sBinData, const std::string &sSep,
                    size_t lines)
{
    return bin2str((const void *)sBinData.data(), sBinData.length(), sSep,
                   lines);
}

int str2bin(const char *psAsciiData, unsigned char *sBinData, int iBinSize)
{
    int iAsciiLength = strlen(psAsciiData);

    int iRealLength =
        (iAsciiLength / 2 > iBinSize) ? iBinSize : (iAsciiLength / 2);
    for (int i = 0; i < iRealLength; i++) {
        sBinData[i] = x2c(psAsciiData + i * 2);
    }
    return iRealLength;
}

std::string str2bin(const std::string &sString, const std::string &sSep,
                    size_t lines)
{
    const char *psAsciiData = sString.c_str();

    int iAsciiLength = sString.length();
    std::string sBinData;
    for (int i = 0; i < iAsciiLength; i++) {
        sBinData += x2c(psAsciiData + i);
        i++;
        i += sSep.length(); //过滤掉分隔符

        if (lines != 0 && sBinData.length() % lines == 0) {
            i++; //过滤掉回车
        }
    }

    return sBinData;
}

char x2c(const std::string &sWhat)
{
    char digit;

    if (sWhat.length() < 2) {
        return '\0';
    }

    digit =
        (sWhat[0] >= 'A' ? ((sWhat[0] & 0xdf) - 'A') + 10 : (sWhat[0] - '0'));
    digit *= 16;
    digit +=
        (sWhat[1] >= 'A' ? ((sWhat[1] & 0xdf) - 'A') + 10 : (sWhat[1] - '0'));

    return (digit);
}

std::string replace(const std::string &sString, const std::string &sSrc,
                    const std::string &sDest)
{
    if (sSrc.empty()) {
        return sString;
    }

    std::string sBuf = sString;

    std::string::size_type pos = 0;

    while ((pos = sBuf.find(sSrc, pos)) != std::string::npos) {
        sBuf.replace(pos, sSrc.length(), sDest);
        pos += sDest.length();
    }

    return sBuf;
}

std::string replace(const std::string &sString,
                    const std::map<std::string, std::string> &mSrcDest)
{
    if (sString.empty()) {
        return sString;
    }

    std::string tmp = sString;
    std::map<std::string, std::string>::const_iterator it = mSrcDest.begin();

    while (it != mSrcDest.end()) {

        std::string::size_type pos = 0;
        while ((pos = tmp.find(it->first, pos)) != std::string::npos) {
            tmp.replace(pos, it->first.length(), it->second);
            pos += it->second.length();
        }

        ++it;
    }

    return tmp;
}

bool matchperiod(const std::string &s, const std::string &pat)
{
    if (s.empty()) {
        return false;
    }

    if (pat.empty()) {
        return true;
    }

    if (pat.find('*') == std::string::npos) {
        return s == pat;
    }

    std::string::size_type sIndex = 0;
    std::string::size_type patIndex = 0;
    do {
        if (pat[patIndex] == '*') {
            if (s[sIndex] == '.') {
                return false;
            }

            while (sIndex < s.size() && s[sIndex] != '.') {
                ++sIndex;
            }
            patIndex++;
        }
        else {
            if (pat[patIndex] != s[sIndex]) {
                return false;
            }
            ++sIndex;
            ++patIndex;
        }
    } while (sIndex < s.size() && patIndex < pat.size());

    return sIndex == s.size() && patIndex == pat.size();
}

bool matchperiod(const std::string &s, const std::vector<std::string> &pat)
{
    for (size_t i = 0; i < pat.size(); i++) {
        if (matchperiod(s, pat[i])) {
            return true;
        }
    }
    return false;
}

void daemon()
{
    pid_t pid;

    if ((pid = fork()) != 0) {
        exit(0);
    }

    setsid();

    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    ignorepipe();

    if ((pid = fork()) != 0) {
        //父进程结束,变成daemon
        exit(0);
    }

    umask(0);

    //  chdir("/");
}

void ignorepipe()
{
    struct sigaction sig;

    memset(&sig, 0, sizeof(struct sigaction));

    sig.sa_handler = SIG_IGN;
    sig.sa_flags = 0;
    sigemptyset(&sig.sa_mask);
    sigaction(SIGPIPE, &sig, nullptr);
}

bool isprimenumber(size_t n)
{
    size_t nn = (size_t)sqrt((double)n);
    for (size_t i = 2; i < nn; i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

size_t tosize(const std::string &s, size_t iDefaultSize)
{
    if (s.empty()) {
        return iDefaultSize;
    }

    char c = s[s.length() - 1];
    if (c != 'K' && c != 'M' && c != 'G' &&
        trim(s) == to_string(strto<size_t>(s))) {
        //没有后缀, 且转换是正确的
        return (size_t)(strto<size_t>(s));
    }
    else if (c == 'K' || c == 'M' || c == 'G') {
        if (s.length() == 1) {
            return iDefaultSize;
        }

        float n = strto<float>(s.substr(0, s.length() - 1));

        if (trim(s.substr(0, s.length() - 1)) != to_string(n)) {
            return iDefaultSize;
        }

        if (c == 'K') {
            return (size_t)(n * 1024);
        }
        if (c == 'M') {
            return (size_t)(n * 1024 * 1024);
        }
        if (c == 'G') {
            return (size_t)(n * 1024 * 1024 * 1024);
        }
    }

    return iDefaultSize;
}

// Generate the randome std::string, a SHA1-sized random number
void getrandomhexchars(char *p, unsigned int len)
{
    const char *const chars = "0123456789abcdef";
    FILE *fp = fopen("/dev/urandom", "r");
    if (!fp || fread(p, len, 1, fp) == 0) {
        for (unsigned int j = 0; j < len; j++)
            p[j] ^= rand();
    }

    if (fp) fclose(fp);

    for (unsigned int j = 0; j < len; j++)
        p[j] = chars[p[j] & 0x0F];
}


// Turns "  hello " into "hello". Also handles tabs.
std::string stripspaces(const std::string &str)
{
    const size_t s = str.find_first_not_of(" \t\r\n");

    if (str.npos != s)
        return str.substr(s, str.find_last_not_of(" \t\r\n") - s + 1);
    else
        return "";
}

// "\"hello\"" is turned to "hello"
// This one assumes that the string has already been space stripped in both
// ends, as done by StripSpaces above, for example.
std::string stripquotes(const std::string &s)
{
    if (s.size() && '\"' == s[0] && '\"' == *s.rbegin())
        return s.substr(1, s.size() - 2);
    else
        return s;
}

bool chararray_from_formatv(char *out, int outsize, const char *format,
                            va_list args)
{
    int writtenCount;

#ifdef _WIN32
    // You would think *printf are simple, right? Iterate on each character,
    // if it's a format specifier handle it properly, etc.
    //
    // Nooooo. Not according to the C standard.
    //
    // According to the C99 standard (7.19.6.1 "The fprintf function")
    //     The format shall be a multibyte character sequence
    //
    // Because some character encodings might have '%' signs in the middle of
    // a multibyte sequence (SJIS for example only specifies that the first
    // byte of a 2 byte sequence is "high", the second byte can be anything),
    // printf functions have to decode the multibyte sequences and try their
    // best to not screw up.
    //
    // Unfortunately, on Windows, the locale for most languages is not UTF-8
    // as we would need. Notably, for zh_TW, Windows chooses EUC-CN as the
    // locale, and completely fails when trying to decode UTF-8 as EUC-CN.
    //
    // On the other hand, the fix is simple: because we use UTF-8, no such
    // multibyte handling is required as we can simply assume that no '%' char
    // will be present in the middle of a multibyte sequence.
    //
    // This is why we look up the default C locale here and use _vsnprintf_l.
    static _locale_t c_locale = nullptr;
    if (!c_locale) c_locale = _create_locale(LC_ALL, "C");
    writtenCount = _vsnprintf_l(out, outsize, format, c_locale, args);
#else
#if !defined(ANDROID) && !defined(__HAIKU__) && !defined(__OpenBSD__)
    // locale_t previousLocale = uselocale( GetCLocale() );
#endif
    writtenCount = vsnprintf(out, outsize, format, args);
#if !defined(ANDROID) && !defined(__HAIKU__) && !defined(__OpenBSD__)
    // uselocale( previousLocale );
#endif
#endif

    if (writtenCount > 0 && writtenCount < outsize) {
        out[writtenCount] = '\0';
        return true;
    }
    else {
        out[outsize - 1] = '\0';
        return false;
    }
}

std::string string_from_format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::string res = string_from_formatv(format, args);
    va_end(args);
    return res;
}

std::string string_from_formatv(const char *format, va_list args)
{
    char *buf = nullptr;
#ifdef _WIN32
    int required = _vscprintf(format, args);
    buf = new char[required + 1];
    CharArrayFromFormatV(buf, required + 1, format, args);

    std::string temp = buf;
    delete[] buf;
#else
#if !defined(ANDROID) && !defined(__HAIKU__) && !defined(__OpenBSD__)
    // locale_t previousLocale = uselocale( GetCLocale() );
#endif
    if (vasprintf(&buf, format, args) < 0) {
        // LOGE( "Unable to allocate memory for string" );
        buf = nullptr;
    }

#if !defined(ANDROID) && !defined(__HAIKU__) && !defined(__OpenBSD__)
    // uselocale( previousLocale );
#endif

    std::string temp = buf;
    free(buf);
#endif
    return temp;
}

std::string join_strings(const std::vector<std::string> &strings,
                         const std::string &delimiter)
{
    // Check if we can return early, just for speed
    return to_string(strings.cbegin(), strings.cend(), delimiter);
}
//} // namespace common


} // namespace util
