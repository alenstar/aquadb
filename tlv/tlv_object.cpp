#include "tlv_object.h"
#include "tlv.h"
#include "tlv_value.h"
#include "util/common.h"
#include "varint.h"
#include <string.h>

namespace tlv {
TlvArray::~TlvArray()
{
    for (auto v : _values) {
        if (v.dtype() == 100) {
            auto p = v.as_object<TlvObject>();
            delete p;
            v.clear();
        }
        else if (v.dtype() == 101) {
            auto p = v.as_object<TlvArray>();
            delete p;
            v.clear();
        }
    }
    _values.clear();
}

int TlvArray::serialize(uint16_t tag, std::string &out) const
{
    if (_values.empty()) {
        // 空对象不编码
        return 0;
    }
    uint8_t wtype = TLV_LTYPE_ARRAY0;
    char taglen[2] = {0x00};
    size_t pos = 0;
    if (tag < 0x08) {
        taglen[0] = (tag << 4) | wtype;
        out.append(taglen, 1);
        pos += 1;
    }
    else if (tag < 0x800) // 2048
    {
        taglen[0] = (tag & 0x007f) | 0x80;
        taglen[1] = ((tag >> 8) & 0x00ff) | wtype;
        out.append(taglen, 2);
        pos += 2;
    }
    else {
        // tag number is too big [0~2047]
        // error
        // TODO
        return -1;
    }
    // write vector size
    {
        size_t len = varint_len_i(static_cast<int64_t>(_values.size()));
        out.append(len, '\0');
        len = varint_encode(
            const_cast<uint8_t *>(
                reinterpret_cast<const uint8_t *>(out.data()) + pos),
            9, static_cast<int64_t>(_values.size()));
        pos += len;
    }
    // write vector item
    out.append(_values.size(), '\0');
    for (auto &v : _values) {
        // TODO
        switch (v.dtype()) {
        case 0:
        case 1:
        case 2:
        case 3: {
            // 数组内tag设置0
            v.serialize(0, out);
        } break;
        case 100: // object
        {
            auto obj = v.as_object<TlvObject>();
            obj->serialize(static_cast<uint16_t>(0), out);
        } break;
        case 101: // array
        {
            auto obj = v.as_object<TlvArray>();
            obj->serialize(static_cast<uint16_t>(0), out);
        } break;
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
int TlvArray::deserialize(const std::string &in) const
{
    // TODO
    uint16_t tag = 0;
    BytesBuffer buf(reinterpret_cast<uint8_t *>(const_cast<char *>(in.data())),
                    in.size());
    return deserialize(&buf, tag);
}

int TlvArray::deserialize(BytesBuffer *in, uint16_t &tag) const
{
    // TODO
    return -1;
}

TlvObject::TlvObject() {}

TlvObject::~TlvObject()
{
    // std::map<int,Tlv*>::iterator itor;
    // for(itor=mTlvMap.begin(); itor != mTlvMap.end(); itor++) {
    //    delete itor->second;
    //}
    for (auto v : _values) {
        if (v.second.dtype() == 100) {
            auto p = v.second.as_object<TlvObject>();
            delete p;
            v.second.clear();
        }
        else if (v.second.dtype() == 101) {
            auto p = v.second.as_object<TlvArray>();
            delete p;
            v.second.clear();
        }
    }
    _values.clear();
}

int TlvObject::serialize(uint16_t tag, std::string &out) const
{
    if (_values.empty()) {
        // 空对象不编码
        return 0;
    }
    uint8_t wtype = TLV_LTYPE_OBJECT;
    char taglen[2] = {0x00};
    size_t pos = 0;
    if (tag < 0x08) {
        taglen[0] = (tag << 4) | wtype;
        out.append(taglen, 1);
        pos += 1;
    }
    else if (tag < 0x800) // 2048
    {
        taglen[0] = (tag & 0x007f) | 0x80;
        taglen[1] = (((tag >> 7) & 0x000f) << 4) | wtype;
        out.append(taglen, 2);
        pos += 2;
    }
    else {
        // tag number is too big [0~2047]
        // error
        // TODO
        return -1;
    }
    // write vector size
    {
        // size_t len = varint_len_i(static_cast<int64_t>(_values.size()));
        // out.append(len, '\0');
        // size_t len =
        // varint_encode(const_cast<uint8_t*>(reinterpret_cast<const
        // uint8_t*>(out.data()) + pos), 9,
        // static_cast<int64_t>(_values.size())); pos += len;
    }
    // write vector item
    // out.append(_values.size(), '\0');

    for (auto v : _values) {
        // TODO
        switch (v.second.dtype()) {
        case 0: //
        case 1:
        case 2:
        case 3:
            v.second.serialize(static_cast<uint16_t>(v.first), out);
            break;
        case 100: // object
        {
            auto obj = v.second.as_object<TlvObject>();
            obj->serialize(static_cast<uint16_t>(v.first), out);
        } break;
        case 101: // array
        {
            auto arr = v.second.as_object<TlvArray>();
            arr->serialize(static_cast<uint16_t>(v.first), out);
        } break;
            /* code */
        //    break;
        default:
            break;
        }
    }
    out.append("\0"); // 标识object结束
    return 0;
}

int TlvObject::deserialize(const std::string &in)
{
    if (in.empty()) {
        // 空对象不解码
        return 0;
    }
    uint16_t tag = 0;
    uint8_t *data = reinterpret_cast<uint8_t *>(const_cast<char *>(in.data()));
    BytesBuffer buf(data, in.size());
    uint8_t wtype = TLV_LTYPE_ERROR;
    char taglen[2] = {0x00};
    size_t pos = 0;
    if (data[0] & 0x80) {
        tag = (data[0] & 0x7f) | ((data[1] >> 4) & 0x0f);
        wtype = data[1] & 0x000f;
        pos += 2;
    }
    else {
        tag = (data[0] & 0x70) >> 4;
        wtype = data[0] & 0x000f;
        pos += 1;
    }
    buf.peek(pos);
    // TODO
    // std::string buf = in.substr(pos);
    while (buf.size()) {
        // object end
        if (*(buf.data()) == 0x00) {
            break;
        }
        TlvValue value;
        uint16_t subtag = 0;
        int rc = value.deserialize(&buf, subtag);
        if (rc != 0) {
            // deserialize error
            return rc;
        }
        _values.emplace(static_cast<int>(subtag), std::move(value));
    }

    return 0;
}

int TlvObject::deserialize(BytesBuffer *in, uint16_t &tag)
{
    if (in->size() == 0) {
        // 空对象不解码
        return 0;
    }
    uint8_t *data = in->data();
    uint8_t wtype = TLV_LTYPE_ERROR;
    char taglen[2] = {0x00};
    size_t pos = 0;
    if (data[0] & 0x80) {
        tag = (data[0] & 0x7f) | ((data[1] >> 4) & 0x0f);
        wtype = data[1] & 0x000f;
        pos += 2;
    }
    else {
        tag = (data[0] & 0x70) >> 4;
        wtype = data[0] & 0x000f;
        pos += 1;
    }
    in->peek(pos);
    // TODO
    // std::string buf = in.substr(pos);
    while (in->size()) {
        if (*(in->data()) == 0x00) {
            break;
        }
        TlvValue value;
        uint16_t subtag = 0;
        value.deserialize(in, tag);
        _values.emplace(static_cast<int>(subtag), std::move(value));
    }
    return 0;
}

} // namespace tlv
