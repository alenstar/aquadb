#define SPDLOG_TAG "TLV"
#include "tlv_value.h"
#include "encoder.h"
#include "tlv_object.h"
#include "util/common.h"
#include "util/logdef.h"
#include "varint.h"

template <class T> inline std::ostream &operator<<(std::ostream &oss, const std::vector<T> &v)
{
    bool firstFlag = true;
    oss << "[";
    std::for_each(begin(v), end(v), [&oss, &firstFlag](const T &t) {
        if (firstFlag)
            firstFlag = false;
        else
            oss << ",";
        oss << t;
    });

    oss << "]";

    return oss;
}

/** Raw memory casts helper */
union _tlv_float_helper {
    float as_float;
    uint32_t as_uint;
};

/** Raw memory casts helper */
union _tlv_double_helper {
    double as_double;
    uint64_t as_uint;
};

/////////////////////////////////////////////////////////////////////
namespace tlv
{
//////////////////////////////////////////////////////////////////////////

TlvValue::TlvValue() : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) {}
TlvValue::TlvValue(int v) : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) { set_value(v); }
TlvValue::TlvValue(int64_t v) : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) { set_value(v); }
TlvValue::TlvValue(uint64_t v) : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) { set_value(v); }
TlvValue::TlvValue(uint32_t v) : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) { set_value(v); }
TlvValue::TlvValue(double v) : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) { set_value(v); }
TlvValue::TlvValue(const std::string &v) : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) { set_value(v); }
TlvValue::TlvValue(const char* v, size_t size) : _sval(nullptr), _size(0), _dtype(0), _iscopy(0) { set_value(v,size); }

TlvValue::TlvValue(const TlvValue &cv)
{
    _size = cv._size;
    _dtype = cv._dtype;
    if (cv._iscopy)
    {
        _sval = reinterpret_cast<char *>(malloc(cv._size + 1));
        memcpy(_sval, cv._sval, cv._size);
        _sval[cv._size] = '\0';
        _iscopy = 1;
    }
    else
    {
        memcpy(_buffer, cv._buffer, sizeof(int64_t) + sizeof(int32_t));
        _iscopy = 0;
    }
}
TlvValue::TlvValue(TlvValue &&cv)
{
    _size = cv._size;
    _dtype = cv._dtype;
    if (cv._iscopy)
    {
        _iscopy = 1;
        _sval = cv._sval;
    }
    else
    {
        _iscopy = 0;
        memcpy(_buffer, cv._buffer, sizeof(int64_t) + sizeof(int32_t));
    }
    cv._sval = nullptr;
    cv._iscopy = 0;
    cv._size = 0;
    cv._dtype = 0;
}

TlvValue &TlvValue::operator=(const TlvValue &cv)
{
    if (this == &cv)
    {
        return *this;
    }

    _size = cv._size;
    _dtype = cv._dtype;
    if (_iscopy)
    {
        free(_sval);
    }
    if (cv._iscopy)
    {
        _sval = reinterpret_cast<char *>(malloc(cv._size + 1));
        memcpy(_sval, cv._sval, cv._size);
        _sval[cv._size] = '\0';
        _iscopy = 1;
    }
    else
    {
        memcpy(_buffer, cv._buffer, sizeof(int64_t) + sizeof(int32_t));
        _iscopy = 0;
    }
    return *this;
}
TlvValue &TlvValue::operator=(TlvValue &&cv) noexcept
{
    if (this == &cv)
    {
        return *this;
    }

    _size = cv._size;
    _dtype = cv._dtype;
    if (_iscopy)
    {
        free(_sval);
    }

    if (cv._iscopy)
    {
        _iscopy = 1;
        _sval = cv._sval;
    }
    else
    {
        _iscopy = 0;
        memcpy(_buffer, cv._buffer, sizeof(int64_t) + sizeof(int32_t));
    }
    cv._sval = nullptr;
    cv._iscopy = 0;
    cv._size = 0;
    cv._dtype = 0;
    return *this;
}

TlvValue::~TlvValue() { clear(); }

void TlvValue::set_value(const std::string &v)
{
    if (v.size() >= 0x0ffffff)
    {
        throw std::invalid_argument("string too big");
    }

    if (_iscopy)
    {
        free(_sval);
    }
    _sval = nullptr;

    if (v.size() < (sizeof(int64_t) + sizeof(int32_t)))
    {
        memcpy(_buffer, v.data(), v.size() + 1);
        _iscopy = 0;
    }
    else
    {
        _sval = reinterpret_cast<char *>(malloc(v.size() + 1));
        memcpy(_sval, v.data(), v.size());
        _sval[v.size()] = '\0';
        _iscopy = 1;
    }
    _size = v.size();
    _dtype = 3;
}

void TlvValue::set_value(const char *v, size_t size)
{
    if (size >= 0x0ffffff)
    {
        throw std::invalid_argument("string too big");
    }

    if (_iscopy)
    {
        free(_sval);
    }
    _sval = nullptr;

    if (size < (sizeof(int64_t) + sizeof(int32_t)))
    {
        memcpy(_buffer, v, size + 1);
        _iscopy = 0;
    }
    else
    {
        _sval = reinterpret_cast<char *>(malloc(size + 1));
        memcpy(_sval, v, size);
        _sval[size] = '\0';
        _iscopy = 1;
    }
    _size = size;
    _dtype = 3;
}

int TlvValue::to_int() const
{
    switch (_dtype)
    {
    case 1:
        return static_cast<int>(_fval);
    case 2:
        return static_cast<int>(_ival);
    case 3:
        return util::strto<int>(c_str());
    default:
        break;
    }
    return 0;
}
int64_t TlvValue::to_long() const
{
    switch (_dtype)
    {
    case 1:
        return static_cast<int64_t>(_fval);
    case 2:
        return _ival;
    case 3:
        return util::strto<int64_t>(c_str());
    default:
        break;
    }
    return 0;
}

/*
int64_t TlvValue::to_ulong() const
{
    switch (_dtype)
    {
    case 1:
        return static_cast<uint64_t>(_fval);
    case 2:
        return _uval;
    case 3:
        return util::strto<uint64_t>(c_str());
    default:
        break;
    }
    return 0;
}
*/

double TlvValue::to_double() const
{
    switch (_dtype)
    {
    case 1:
        return _fval;
    case 2:
        return static_cast<double>(_ival);
    case 3:
        return util::strto<double>(c_str());
    default:
        // return std::numeric_limits<double>::quiet_NaN();
        break;
    }
    return 0.0;
}
std::string TlvValue::to_string() const
{
    switch (_dtype)
    {
    case 1:
        return util::to_string(_fval);
    case 2:
        return util::to_string(_ival);
    case 3:
        return c_str();
    default:
        break;
    }
    return "";
}

int TlvValue::serialize(uint16_t tag, std::vector<uint8_t> &out) const
{
    int rc = 0;
    uint8_t wtype = wrie_type();
    uint8_t taglen[2] = {0x00};
    LOGD("tag=%d,wtype=%d, size=%lu,%lu", tag, wtype, out.size(), size());
    if (tag < 0x08)
    {
        taglen[0] = (tag << 4) | wtype;
        //    LOGD("tag=%d,wtype=%d, size=%lu,%lu %u", tag, wtype, out.size(), size(), taglen[0]);
        out.push_back(taglen[0]);
    }
    else if (tag < 0x800) // 2048
    {
        taglen[0] = (tag & 0x007f) | 0x80;
        taglen[1] = ((tag >> 8) & 0x00ff) | wtype;
        out.push_back(taglen[0]);
        out.push_back(taglen[1]);
    }
    else
    {
        // tag number is too big [0~2047]
        // error
        // TODO
        return -1;
    }
    switch (wtype)
    {
    case TLV_LTYPE_NULL: // /* null */
        /* code */
        LOGD("is null or none");
        break;
    case TLV_LTYPE_TRUE: /* true, 1 */
        LOGD("is true or 1");
        break;
    case TLV_LTYPE_FALSE: /* false, 0 */
        LOGD("is false or 0");
        break;
    case TLV_LTYPE_NAN: /* NaN */
        break;
    case TLV_LTYPE_ONE_NEG: /* -1 */
        break;
    case TLV_LTYPE_VARINT_POS: /* integer > 0 */
        // encode varint
        {
            size_t len = varint_len_i(to_long());
            // LOGD("varint %l, size=%lu", to_long(), len);
            out.resize(out.size() + len);
            rc = varint_encode(const_cast<uint8_t *>(out.data() + out.size() - len), 9, to_long());
            // LOGD("varint %l, size=%lu, rc=%lu", to_long(), len,rc);
        }
        break;
    case TLV_LTYPE_VARINT_NEG: /* integer < 0 */
        // encode varint abs(neg_number)
        {
            int64_t v = std::abs(to_long());
            size_t len = varint_len_i(v);
            // LOGD("varint %l, size=%lu", to_long(), len);
            out.resize(out.size() + len);
            rc = varint_encode(const_cast<uint8_t *>(out.data() + out.size() - len), 9, v);
            // LOGD("varint %l, size=%lu, rc=%lu", to_long(), len,rc);
        }
        break;
    case TLV_LTYPE_FLOAT32: /* float */
    {
        // TODO
        // encode for float
        out.resize(out.size() + sizeof(float));
        uint32_t *p = reinterpret_cast<uint32_t *>(out.data() + out.size() - sizeof(float));
        *p = KeyEncoder::to_little_endian_u32(
            ((union _tlv_float_helper){.as_float = static_cast<float>(to_double())}).as_uint);
    }
    break;
    case TLV_LTYPE_FLOAT64: /* double */
    {
        // TODO
        // encode for double
        out.resize(out.size() + sizeof(double));
        // LOGD("tag=%d, len=%lu, size=%lu",tag, sizeof(double), size());
        uint64_t *p = reinterpret_cast<uint64_t *>(out.data() + out.size() - sizeof(double));
        *p = KeyEncoder::to_little_endian_u64(((union _tlv_double_helper){.as_double = to_double()}).as_uint);
    }
    break;
    case TLV_LTYPE_BYTES: /* bytes or array<uint8>, array<int8> string (non-terminal '\0' ) */
    {
        // encode varint
        size_t len = varint_len_i(size());
        out.resize(out.size() + len + size());
        // LOGD("tag=%d, len=%lu, size=%lu",tag, len, size());
        varint_encode(const_cast<uint8_t *>(out.data() + out.size() - len - size()), 9, size());
        ::memcpy(out.data() + out.size() - size(), data(), size());
    }
    break;
    case TLV_LTYPE_ARRAY0: /* any type array, array_size + elem_type + elem_data[n] */
    {
        if (dtype() != 101)
        {
            LOGE("bad dtype:%d for array", dtype());
            return -1;
        }
        auto obj = as_object<TlvArray>();
        rc = obj->serialize(static_cast<uint16_t>(0), out);
        if (rc != 0)
        {
            return rc;
        }
    }
    break;
    case TLV_LTYPE_OBJECT: /* object object_size + object_data */
    {
        if (dtype() != 100)
        {
            LOGE("bad dtype:%d for array", dtype());
            return -1;
        }
        auto obj = as_object<TlvObject>();
        rc = obj->serialize(static_cast<uint16_t>(0), out);
        if (rc != 0)
        {
            return rc;
        }
    }
    break;
    case TLV_LTYPE_ARRAY2: /* array<int16> or array<uint16> array_size + elem_data[n] */
    // break;
    case TLV_LTYPE_ARRAY4: /* array<int32> or array<uint32> array_size + elem_data[n] */
    // break;
    case TLV_LTYPE_ARRAY8: /* array<int64> or array<double> array_size + elem_data[n] */
    // break;
    case TLV_LTYPE_ERROR: /* error type */
        break;
    default:
        // unknown wtype
        // error
        // TODO
        return -1;
    }

    return 0;
}

// int TlvValue::deserialize(const std::string &in)
//{
//    uint16_t tag = 0;
//    BytesBuffer buf(reinterpret_cast<uint8_t*>(const_cast<char*>(in.data())), in.size());
//    return deserialize(&buf,tag);
//}

int TlvValue::deserialize(BytesBuffer *in, uint16_t &tag)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(in->data());
    uint8_t wtype = TLV_LTYPE_ERROR;
    char taglen[2] = {0x00};
    size_t pos = 0;
    if (data[0] & 0x80)
    {
        tag = (data[0] & 0x7f) | ((data[1] >> 4) & 0x0f);
        wtype = data[1] & 0x000f;
        pos += 2;
    }
    else
    {
        tag = (data[0] & 0x70) >> 4;
        wtype = data[0] & 0x000f;
        pos += 1;
    }
    data = in->peek(pos);
    // TODO
    switch (wtype)
    {
    case TLV_LTYPE_NULL: // /* null */
        /* code */
        break;
    case TLV_LTYPE_TRUE: /* true, 1 */
        set_value(1);
        break;
    case TLV_LTYPE_FALSE: /* false, 0 */
        set_value(0);
        break;
    case TLV_LTYPE_NAN: /* NaN */
        set_value(std::numeric_limits<double>::quiet_NaN());
        break;
    case TLV_LTYPE_ONE_NEG:
        set_value(-1);
        break;
    // case TLV_LTYPE_INF_POS: /* +Inf */
    //    break;
    // case TLV_LTYPE_INF_NEG: /* -Inf */
    //    break;
    case TLV_LTYPE_VARINT_POS: /* integer > 0 */
        // encode varint
        {
            int64_t v = 0;
            size_t len = varint_decode(&v, data, in->size() < 9 ? in->size() : 9);
            in->peek(len);
            pos += len;
            set_value(v);
        }
        break;
    case TLV_LTYPE_VARINT_NEG: /* integer < 0 */
        // encode varint abs(neg_number)
        {
            int64_t v = 0;
            size_t len = varint_decode(&v, data, in->size() < 9 ? in->size() : 9);
            in->peek(len);
            pos += len;
            set_value(v * (-1));
        }
        break;
    case TLV_LTYPE_FLOAT32: /* float */
    {
        float v = ((union _tlv_float_helper){
                       .as_uint = KeyEncoder::to_little_endian_u32(*reinterpret_cast<uint32_t *>(in->data()))})
                      .as_float;
        pos += sizeof(v);
        in->peek(sizeof(v));
        set_value(v);
    }
    break;
    case TLV_LTYPE_FLOAT64: /* double */
    {
        double v = ((union _tlv_double_helper){
                        .as_uint = KeyEncoder::to_little_endian_u64(*reinterpret_cast<uint64_t *>(in->data()))})
                       .as_double;
        pos += sizeof(v);
        in->peek(sizeof(v));
        set_value(v);
    }
    break;
    case TLV_LTYPE_BYTES: /* bytes or array<uint8>, array<int8> string (non-terminal '\0' ) */
    {
        // encode varint
        int64_t sz = 0;
        size_t len = varint_decode(&sz, data, in->size() < 9 ? in->size() : 9);
        in->peek(len);
        pos += len;
        set_value(reinterpret_cast<char *>(in->data()), sz);
        pos += sz;
        in->peek(sz);
    }
    break;
    case TLV_LTYPE_ARRAY0: /* any type array, array_size + elem_type + elem_data[n] */
    // break;
    case TLV_LTYPE_ARRAY2: /* array<int16> or array<uint16> array_size + elem_data[n] */
    // break;
    case TLV_LTYPE_ARRAY4: /* array<int32> or array<uint32> array_size + elem_data[n] */
    // break;
    case TLV_LTYPE_ARRAY8: /* array<int64> or array<double> array_size + elem_data[n] */
    // break;
    case TLV_LTYPE_OBJECT: /* object object_size + object_data */
    // break;
    case TLV_LTYPE_ERROR: /* error type */
        break;
    default:
        // unknown wtype
        return false;
    }

    return false;
}

std::ostream &operator<<(std::ostream &oss, const TlvValue &o)
{
    if (o.is_none())
    {
        oss << "<none>";
        return oss;
    }

    switch (o._dtype)
    {
    case 1:
        oss << o.to_double();
        break;
    case 2:
        oss << o.to_long();
        break;
    case 3:
        oss << o.c_str();
        break;
    default:
        break;
    }
    return oss;
}

} // namespace tlv
