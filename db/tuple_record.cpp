#define SPDLOG_TAG "TLV"
#include "tuple_record.h"
#include "tuple_value.h"
#include "util/common.h"
#include "util/logdef.h"
#include "varint.h"
#include <string.h>

namespace aquadb
{

Array::Array() {}

Array::Array(Array &&a) { _values = std::move(a._values); }

Array::~Array()
{
    for (auto v : _values)
    {
        if (v.dtype() == 100)
        {
            auto p = v.as_object<TupleRecord>();
            delete p;
            v.clear();
        }
        else if (v.dtype() == 101)
        {
            auto p = v.as_object<Array>();
            delete p;
            v.clear();
        }
    }
    _values.clear();
}

int Array::serialize(uint16_t tag, std::vector<uint8_t> &out) const
{
    if (_values.empty())
    {
        // 空对象不编码
        return 0;
    }
    uint8_t wtype = TLV_LTYPE_ARRAY0;
    uint8_t taglen[2] = {0x00};
    size_t pos = 0;
    if (tag < 0x08)
    {
        taglen[0] = (tag << 4) | wtype;
        out.push_back(taglen[0]);
        pos += 1;
    }
    else if (tag < 0x800) // 2048
    {
        taglen[0] = (tag & 0x007f) | 0x80;
        taglen[1] = ((tag >> 8) & 0x00ff) | wtype;
        out.push_back(taglen[0]);
        out.push_back(taglen[1]);
        pos += 2;
    }
    else
    {
        // tag number is too big [0~2047]
        // error
        // TODO
        return -1;
    }
    // write vector size
    {
        size_t len = varint_len_i(static_cast<int64_t>(_values.size()));
        out.resize(out.size() + len);
        varint_encode(const_cast<uint8_t *>(out.data() + out.size() - len), 9, static_cast<int64_t>(_values.size()));
    }
    // write vector item
    for (auto &v : _values)
    {
        // TODO
        switch (v.dtype())
        {
        case 0:
        case 1:
        case 2:
        case 3:
        {
            // 数组内tag设置0
            v.serialize(0, out);
        }
        break;
        case 100: // object
        {
            auto obj = v.as_object<TupleRecord>();
            obj->serialize(static_cast<uint16_t>(0), out);
        }
        break;
        case 101: // array
        {
            auto obj = v.as_object<Array>();
            obj->serialize(static_cast<uint16_t>(0), out);
        }
        break;
        case 91: // vector<int8>
            break;
        case 92: // vector<uint8>
            break;
        case 93: // vector<int16>
            break;
        case 94: // vector<uint16>
            break;
        case 95: // vector<int32>
            break;
        case 96: // vector<uint32>
            break;
        case 97: // vector<int64>
            break;
        case 98: // vector<uint64>
            break;
        case 99: // vector<double>
            break;
            // case TLV_LTYPE_ERROR:
            /* code */
        //    break;
        default:
            break;
        }
    }
    return 0;
}
int Array::deserialize(const std::string &in) 
{
    // TODO
    uint16_t tag = 0;
    BytesBuffer buf(reinterpret_cast<uint8_t *>(const_cast<char *>(in.data())), in.size());
    return deserialize(&buf, tag);
}

int Array::deserialize(BytesBuffer *in, uint16_t &tag)
{
    // TODO
    if (in->size() == 0)
    {
        // 空对象不解码
        return 0;
    }
    uint8_t *data = in->data();
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
    LOGD("tag=%d, wtype=%d, pos=%lu", tag, wtype, pos);
    in->peek(pos);
    int64_t size = 0;
    // read vector size
    {
        size_t len = varint_decode(&size, const_cast<uint8_t *>(in->data()), 9);
        in->peek(len);
    }
    // TODO
    // std::string buf = in.substr(pos);
    int rc = 0;
    for (int64_t i = 0; i < size; ++i)
    {
        Value value;
        uint16_t subtag = 0;
        rc = value.deserialize(in, subtag);
        if (rc != 0)
        {
            LOGE("bad format: tag=%d, value=%s", subtag, value.to_string().c_str());
            break;
        }
        LOGD("tag=%d, dtype=%d, value=%s", subtag, value.dtype(), value.to_string().c_str());
        _values.emplace_back(std::move(value));
    }
    return 0;
}

TupleRecord::TupleRecord() {}
TupleRecord::TupleRecord(TupleRecord &&o) { _values = std::move(o._values); }

TupleRecord::~TupleRecord()
{
    for (auto v : _values)
    {
        if (v.second.dtype() == 100)
        {
            auto p = v.second.as_object<TupleRecord>();
            delete p;
            v.second.clear();
        }
        else if (v.second.dtype() == 101)
        {
            auto p = v.second.as_object<Array>();
            delete p;
            v.second.clear();
        }
    }
    _values.clear();
}

int TupleRecord::serialize(uint16_t tag, std::vector<uint8_t> &out) const
{
    if (_values.empty())
    {
        // 空对象不编码
        return 0;
    }
    // out.reserve(out.size() + _values.size() * 8);
    uint8_t wtype = TLV_LTYPE_OBJECT;
    uint8_t taglen[2] = {0x00};
    if (tag < 0x08)
    {
        taglen[0] = (tag << 4) | wtype;
        out.push_back(taglen[0]);
    }
    else if (tag < 0x800) // 2048
    {
        taglen[0] = (tag & 0x007f) | 0x80;
        taglen[1] = (((tag >> 7) & 0x000f) << 4) | wtype;
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
    int rc = 0;
    for (auto v : _values)
    {
        // TODO

        LOGD("tag=%d, dtype=%d, value=%s, size=%lu,%lu", v.first, v.second.dtype(), v.second.to_string().c_str(),
             out.size(), v.second.size());
        switch (v.second.dtype())
        {
        case 0: //
        case 1:
        case 2:
        case 3:
            rc = v.second.serialize(static_cast<uint16_t>(v.first), out);
            if (rc != 0)
            {
                LOGE("serialize fail:tag=%d, dtype=%d, value=%s, size=%lu,%lu", v.first, v.second.dtype(),
                     v.second.to_string().c_str(), out.size(), v.second.size());
                return rc;
            }
            break;
        case Value::DataType::OBJECT: // object
        {
            auto obj = v.second.as_object<TupleRecord>();
            rc = obj->serialize(static_cast<uint16_t>(v.first), out);
            if (rc != 0)
            {
                LOGE("serialize fail:tag=%d, dtype=%d, value=%s, size=%lu,%lu", v.first, v.second.dtype(),
                     v.second.to_string().c_str(), out.size(), v.second.size());
                return rc;
            }
        }
        break;
        case Value::DataType::ARRAY: // array
        {
            auto arr = v.second.as_object<Array>();
            rc = arr->serialize(static_cast<uint16_t>(v.first), out);
            if (rc != 0)
            {
                LOGE("serialize fail:tag=%d, dtype=%d, value=%s, size=%lu,%lu", v.first, v.second.dtype(),
                     v.second.to_string().c_str(), out.size(), v.second.size());
                return rc;
            }
        }
        break;
            /* code */
        //    break;
        default:
            break;
        }
    }
    out.push_back('\0'); // 标识object结束
    return 0;
}

int TupleRecord::deserialize(const std::string &in)
{
    if (in.empty())
    {
        // 空对象不解码
        return 0;
    }
    uint16_t tag = 0;
    uint8_t *data = reinterpret_cast<uint8_t *>(const_cast<char *>(in.data()));
    BytesBuffer buf(data, in.size());
    return deserialize(&buf, tag);
}

int TupleRecord::deserialize(const std::vector<uint8_t> &in)
{
    if (in.empty())
    {
        // 空对象不解码
        return 0;
    }
    uint16_t tag = 0;
    uint8_t *data = const_cast<uint8_t *>(in.data());
    BytesBuffer buf(data, in.size());
    return deserialize(&buf, tag);
}

int TupleRecord::deserialize(BytesBuffer *in, uint16_t &tag)
{
    if (in->size() == 0)
    {
        // 空对象不解码
        return 0;
    }
    uint8_t *data = in->data();
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
    LOGD("tag=%d, wtype=%d, pos=%lu", tag, wtype, pos);
    in->peek(pos);
    // TODO
    // std::string buf = in.substr(pos);
    int rc = 0;
    while (in->size())
    {
        Value value;
        uint16_t subtag = 0;
        rc = value.deserialize(in, subtag);
        if (rc != 0)
        {
            LOGE("bad format: tag=%d, value=%s", subtag, value.to_string().c_str());
            break;
        }
        LOGD("tag=%d, dtype=%d, value=%s", subtag, value.dtype(), value.to_string().c_str());
        _values.emplace(static_cast<int>(subtag), std::move(value));
        if (*(in->data()) == 0x00)
        {
            in->peek(1);
            LOGD("object item end");
            break;
        }
    }
    return 0;
}


////////////////////////////////////////

int serialize_bytes(uint16_t tag, const char *values, size_t size,  std::vector<uint8_t> &out)
{
    if (size == 0)
    {
        // 空对象不编码
        return 0;
    }
    //uint8_t wtype = TLV_LTYPE_BYTES;
    char taglen[2] = {0x00};
    // TODO
    return -1;
}

int serialize_array2(uint16_t tag, const std::vector<int16_t> &values,  std::vector<uint8_t> &out)
{
    if (values.empty())
    {
        // 空对象不编码
        return 0;
    }
    //uint8_t wtype = TLV_LTYPE_ARRAY2;
    char taglen[2] = {0x00};
    // TODO
    return -1;
}

int serialize_array2(uint16_t tag, const std::vector<uint16_t> &values,  std::vector<uint8_t> &out) { return -1; }

int serialize_array4(uint16_t tag, const std::vector<int32_t> &values,  std::vector<uint8_t> &out) { return -1; }

int serialize_array4(uint16_t tag, const std::vector<uint32_t> &values,  std::vector<uint8_t> &out) { return -1; }

int serialize_array4(uint16_t tag, const std::vector<float> &values,  std::vector<uint8_t> &out) { return -1; }

int serialize_array8(uint16_t tag, const std::vector<int64_t> &values,  std::vector<uint8_t> &out) { return -1; }

int serialize_array8(uint16_t tag, const std::vector<double> &values,  std::vector<uint8_t> &out) { return -1; }

int serialize_object(uint16_t tag, const std::vector<double> &values,  std::vector<uint8_t> &out) { return -1; }

} // namespace aquadb
