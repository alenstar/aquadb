#pragma once

#include <cstdint>

// tlv line type
#define TLV_LTYPE_NULL    0x00U /* null */
#define TLV_LTYPE_TRUE    0x01U /* true, 1 */
#define TLV_LTYPE_FALSE    0x02U /* false, 0 */
#define TLV_LTYPE_NAN    0x03U /* NaN */
#define TLV_LTYPE_ONE_NEG    0x04U /* -1 */
//#define TLV_LTYPE_INF_POS    0x04U /* +Inf */
//#define TLV_LTYPE_INF_NEG    0x05U /* -Inf */
#define TLV_LTYPE_VARINT_POS  0x06U /* integer > 0 */
#define TLV_LTYPE_VARINT_NEG  0x07U /* integer < 0 */
#define TLV_LTYPE_FLOAT32 0x08U /* float */
#define TLV_LTYPE_FLOAT64 0x09U /* double */
#define TLV_LTYPE_BYTES 0x0aU /* bytes or array<uint8>, array<int8> string (non-terminal '\0' ) */
#define TLV_LTYPE_ARRAY0 0x0bU /* any type array, array_size + elem[elem_type + elem_data, ...] */
#define TLV_LTYPE_ARRAY2 0x0cU /* array<int16> or array<uint16> array_size + elem_data[n] */
#define TLV_LTYPE_ARRAY4 0x0dU /* array<int32> or array<uint32> array_size + elem_data[n] */
#define TLV_LTYPE_ARRAY8 0x0eU /* array<int64> or array<double> array_size + elem_data[n] */
#define TLV_LTYPE_OBJECT 0x0fU /* object object_size + object_data */
#define TLV_LTYPE_ERROR 0xffu /* error type */

// tlv value type
#define TLV_VTYPE_NONE 0
#define TLV_VTYPE_DOUBLE 1
#define TLV_VTYPE_LONG 2
#define TLV_VTYPE_BYTES 3
#define TLV_VTYPE_STRING 3
#define TLV_VTYPE_OBJECT 100
#define TLV_VTYPE_ARRAY 101

// 编码格式
// |head = (flag + tag + type)|data|
// flag = 0 时, head 占一个字节
// flag = 1 时, head 占两个字节

// head=0 编码格式 tag in [0-7]
// | 1bit |3bits   |4bits     |
// | 0    |tag[0-7]|value type|
//
// head=1 编码格式 tag in [8-2000]
// | 1bit |11bits      |4bits     |
// | 1    |tag[8-2047] |value type|

// tlv tag range
// 0 内部扩展用
// 1 - 7 用户使用
// 8 - 2019 用户使用
// 2020 扩展长度用，通过嵌套object(tag=2020)来使tag超出2019的限制,
//      嵌套体中存的tag=用户定义的tag-2019, 例如: 存2020, 实际存的是 2020:{1:{user data}}
// 2021 - 2047 扩展使用


namespace tlv
{
enum MysqlType : uint8_t { 
    MYSQL_TYPE_DECIMAL,   // 0
    MYSQL_TYPE_TINY,
    MYSQL_TYPE_SHORT,  
    MYSQL_TYPE_LONG,
    MYSQL_TYPE_FLOAT,  
    MYSQL_TYPE_DOUBLE,   // 5
    MYSQL_TYPE_NULL,   
    MYSQL_TYPE_TIMESTAMP,
    MYSQL_TYPE_LONGLONG,
    MYSQL_TYPE_INT24,
    MYSQL_TYPE_DATE,     // 10
    MYSQL_TYPE_TIME,
    MYSQL_TYPE_DATETIME, 
    MYSQL_TYPE_YEAR,
    MYSQL_TYPE_NEWDATE, 
    MYSQL_TYPE_VARCHAR,
    MYSQL_TYPE_BIT,
    MYSQL_TYPE_TDIGEST = 242,
    MYSQL_TYPE_BITMAP = 243,
    MYSQL_TYPE_HLL = 244,
    MYSQL_TYPE_JSON = 245,
    MYSQL_TYPE_NEWDECIMAL = 246,
    MYSQL_TYPE_ENUM = 247,
    MYSQL_TYPE_SET = 248,
    MYSQL_TYPE_TINY_BLOB = 249,
    MYSQL_TYPE_MEDIUM_BLOB = 250,
    MYSQL_TYPE_LONG_BLOB = 251,
    MYSQL_TYPE_BLOB = 252,
    MYSQL_TYPE_VAR_STRING = 253,
    MYSQL_TYPE_STRING = 254,
    MYSQL_TYPE_GEOMETRY = 255
};

}