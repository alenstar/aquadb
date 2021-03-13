#pragma once

#include <cstdint>

#define TLV_LTYPE_NULL    0x00U /* null */
#define TLV_LTYPE_TRUE    0x01U /* true, 1 */
#define TLV_LTYPE_FALSE    0x02U /* false, 0 */
#define TLV_LTYPE_NAN    0x03U /* NaN */
#define TLV_LTYPE_INF_POS    0x04U /* +Inf */
#define TLV_LTYPE_INF_NEG    0x05U /* -Inf */
#define TLV_LTYPE_VARINT_POS  0x06U /* integer > 0 */
#define TLV_LTYPE_VARINT_NEG  0x07U /* integer < 0 */
#define TLV_LTYPE_FLOAT32 0x08U /* float */
#define TLV_LTYPE_FLOAT64 0x09U /* double */
#define TLV_LTYPE_BYTES 0x0aU /* bytes or array<uint8>, array<int8> string (non-terminal '\0' ) */
#define TLV_LTYPE_ARRAY0 0x0bU /* any type array, array_size + elem[elem_size + elem_data, ...] */
#define TLV_LTYPE_ARRAY2 0x0cU /* array<int16> or array<uint16> array_size + elem_data[n] */
#define TLV_LTYPE_ARRAY4 0x0dU /* array<int32> or array<uint32> array_size + elem_data[n] */
#define TLV_LTYPE_ARRAY8 0x0eU /* array<int64> or array<double> array_size + elem_data[n] */
#define TLV_LTYPE_OBJECT 0x0fU /* object object_size + object_data */
#define TLV_LTYPE_ERROR 0xffu /* error type */
