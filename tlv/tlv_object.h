#pragma once

#include <map>
#include <string>
#include <tlv_types.h>
#include <tlv_value.h>
#include <vector>

namespace tlv {

class Tlv;
// class TlvValue;
class TlvObject;
// class BytesBuffer;

class TlvArray {
public:
    TlvArray();
    ~TlvArray();

    void append(const TlvValue &v);
    void append(const TlvObject &v);

    int serialize(uint16_t tag, std::string &out) const;
    int deserialize(const std::string &in) const;
    int deserialize(BytesBuffer *in, uint16_t &tag) const;

private:
    std::vector<TlvValue> _values;
};

// TlvObject or TlvObj
class TlvObject {
public:
    TlvObject();
    ~TlvObject();

    int serialize(uint16_t tag, std::string &out) const;
    /**
     * @brief
     *
     * @param in
     * @param tag
     * @return int != 0 fial； == 0  success
     */
    int deserialize(const std::string &in);
    int deserialize(BytesBuffer *in, uint16_t &tag);

private:
    TlvObject(const TlvObject &c);
    TlvObject &operator=(const TlvObject &c);

public:
    // put one TLV object

    // do encode
    // returns number of TLVs in TlvObject, along with a vector of the tags
    int GetTLVList(std::vector<int> &list) const;

private:
    // std::map<int,Tlv*> mTlvMap;
    std::map<int, TlvValue> _values;
    // std::vector<TlvObject*> _boxs; // 记录box对象，用于回收资源
};

} // namespace tlv
