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

    int serialize(uint16_t tag, std::vector<uint8_t> &out) const;
    int deserialize(const std::vector<uint8_t> &in) const;
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

    int serialize(uint16_t tag, std::vector<uint8_t> &out) const;
    inline int serialize(std::vector<uint8_t> &out) const { return serialize(0, out);}
    /**
     * @brief
     *
     * @param in
     * @param tag
     * @return int != 0 fial； == 0  success
     */
    int deserialize(const std::string &in);
    int deserialize(const std::vector<uint8_t> &in);
    int deserialize(BytesBuffer *in, uint16_t &tag);

    int insert(int tag, const TlvValue& v)
    {
        _values[tag] = v;
    }
    int insert(int tag, TlvValue&& v)
    {
        _values[tag] = std::move(v);
    }

    int insert(int tag, const TlvObject& v);
    int insert(int tag, const TlvArray& v);

    TlvValue* get(int tag) 
    {
        auto it = _values.find(tag);
        if(it == _values.cend())
        {
            return nullptr;
        }
        return &(it->second);
    }
    size_t size() const { return _values.size();}
private:
    TlvObject(const TlvObject &c);
    TlvObject &operator=(const TlvObject &c);

public:

private:
    std::map<int, TlvValue> _values;
};

} // namespace tlv
