#pragma once

#include <iostream>
#include <vector>

namespace util {
/////////////////////////////////////////////////
/**
 * @file tc_encoder.h
 * @brief  转码类
 *
 *
 */
/////////////////////////////////////////////////


    /**
     * @brief  gbk 转换到 utf8.
     *
     * @param sOut        输出buffer
     * @param iMaxOutLen  输出buffer最大的长度/sOut的长度
     * @param sIn         输入buffer
     * @param iInLen      输入buffer长度
     * @throws            Encoder_Exception
     * @return
     */
    static int gbk2utf8( char *sOut, int &iMaxOutLen, const char *sIn, int iInLen );

    /**
     * @brief  gbk 转换到 utf8.
     *
     * @param sIn   输入buffer*
     * @throws      Encoder_Exception
     * @return      转换后的utf8编码
     */
    static std::string gbk2utf8( const std::string &sIn );

    /**
     * @brief  gbk 转换到 utf8.
     *
     * @param sIn    输入buffer
     * @param vtStr  输出gbk的vector
     * @throws       Encoder_Exception
     * @return
     */
    static int gbk2utf8( const std::string &sIn, std::vector<std::string> &vtStr );

    /**
     * @brief  utf8 转换到 gbk.
     *
     * @param sOut       输出buffer
     * @param iMaxOutLen 输出buffer最大的长度/sOut的长度
     * @param sIn        输入buffer
     * @param iInLen     输入buffer长度
     * @throws           Encoder_Exception
     * @return
     */
    static int utf82gbk( char *sOut, int &iMaxOutLen, const char *sIn, int iInLen );

    /**
     * @brief  utf8 转换到 gbk.
     *
     * @param sIn  输入buffer
     * @throws     Encoder_Exception
     * @return    转换后的gbk编码
     */
    static std::string utf82gbk( const std::string &sIn );

    /**
     * @brief  将std::string的\n替换掉,转义字符串中的某个字符
     *
     * 缺省:\n 转换为 \r\0; \r转换为\,
     *
     * 主要用于将std::string记录在一行，通常用于写bin-log的地方;
     * @param str   待转换字符串
     * @param f     需要转义的字符
     * @param t     转义后的字符
     * @param u     借用的转义符
     * @return str  转换后的字符串
     */
    static std::string characterTransTo( const std::string &str, char f = '\n', char t = '\r', char u = '\0' );

    /**
     * @brief  从替换的数据恢复源数据,将 transTo 的字符串还原，
     *
     *  缺省:\r\0 还原为\n，\r\r还原为,
     *
     *  主要用于将std::string记录在一行，通常用于写bin-log的地方
     * @param str  待还原的字符串(必须是transTo后得到的字符串)
     * @param f    被转义的字符
     * @param t    转义后的字符
     * @param u    借用的转义符
     * @return str 还原后的字符串
     */
    static std::string characterTransFrom( const std::string &str, char f = '\n', char t = '\r', char u = '\0' );


} // namespace util

