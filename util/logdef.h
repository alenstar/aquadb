#pragma once

#include <easyloggingpp/easylogging++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define def_msleep( M ) usleep( 1000 * ( M ) )
#define def_min( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define def_max( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )

#ifdef _WIN32
#define SPDLOG_FILENAME ( strrchr( __FILE__, '\\' ) ? strrchr( __FILE__, '\\' ) + 1 : __FILE__ )
#else
#define SPDLOG_FILENAME ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )
#endif

#define XLOG_BUFFER_DEPTH ( 32 )
#define XLOG_BUFFER_SIZE ( 4096 )
#define XLOG_BUFFER_CACHE ( 1024 )

#ifndef UNUSED
#define UNUSED( x ) ( (void) ( x ) )
#endif

#define MY_LOG_TYPE 1
// #ifdef __cplusplus
/**
 * TAG 必须使用  SPDLOG_INITEX(tag) 来注册一次,不然会段错误
 * 默认使用debug级别的日志
 * 默认使用多线程支持
 * 默认使用stdout输出日志
 * */
#ifndef SPDLOG_TAG
#define SPDLOG_TAG "default"
#endif

#define LOG_DEBUG( X ) do{LOG( DEBUG ) << X ;}while(0)
#define LOG_INFO( X ) do{LOG( INFO ) << X ;} while(0)
#define LOG_NOTICE( X ) do{LOG( NOTICE ) << X ; }while(0)
#define LOG_WARN( X ) do{LOG( WARNING ) << X ; }while(0)
#define LOG_ERROR( X ) do{LOG( ERROR ) << X ; } while(0)
#define LOG_FATA( X ) do{LOG( FATA ) << X ;}while(0)

#if MY_LOG_TYPE == 1 // def DEBUG
#define LOGD( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        fprintf( stderr, "[%s] [D] %s::%s() %d|%s\n", SPDLOG_TAG, SPDLOG_FILENAME, __FUNCTION__, __LINE__,             \
                 xlog_tmp_buf__ );                                                                                     \
    } while ( 0 )
#define LOGI( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        fprintf( stderr, "[%s] [I] %s::%s() %d|%s\n", SPDLOG_TAG, SPDLOG_FILENAME, __FUNCTION__, __LINE__,             \
                 xlog_tmp_buf__ );                                                                                     \
    } while ( 0 )
#define LOGW( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        fprintf( stderr, "[%s] [W] %s::%s() %d|%s\n", SPDLOG_TAG, SPDLOG_FILENAME, __FUNCTION__, __LINE__,             \
                 xlog_tmp_buf__ );                                                                                     \
    } while ( 0 )
#define LOGE( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        fprintf( stderr, "[%s] [E] %s::%s() %d|%s\n", SPDLOG_TAG, SPDLOG_FILENAME, __FUNCTION__, __LINE__,             \
                 xlog_tmp_buf__ );                                                                                     \
    } while ( 0 )

#elif MY_LOG_TYPE == 2
#define LOGD( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        LOG( DEBUG ) << xlog_tmp_buf__;                                                                                \
    } while ( 0 )
#define LOGI( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        LOG( INFO ) << xlog_tmp_buf__;                                                                                 \
    } while ( 0 )
#define LOGW( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        LOG( WARNING ) << xlog_tmp_buf__;                                                                              \
    } while ( 0 )
#define LOGE( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        LOG( ERROR ) << xlog_tmp_buf__;                                                                                \
    } while ( 0 )

#else
#define LOGV( ... ) ( (void) ( 0 ) )
#define LOGD( ... )
#define LOGI( ... )
#define LOGW( ... )
#define LOGE( ... )                                                                                                    \
    do {                                                                                                               \
        char xlog_tmp_buf__[ XLOG_BUFFER_SIZE ] = {0x00};                                                              \
        snprintf( xlog_tmp_buf__, sizeof( xlog_tmp_buf__ ), ##__VA_ARGS__ );                                           \
        fprintf( stderr, "[%s] [E] %s::%s() %d|%s\n", SPDLOG_TAG, SPDLOG_FILENAME, __FUNCTION__, __LINE__,             \
                 xlog_tmp_buf__ );                                                                                     \
    } while ( 0 )
#endif
