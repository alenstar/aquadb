#pragma once

#include <map>
#include <string>
#include <tlv_types.h>
#include <tlv_value.h>
#include <vector>

namespace tlv
{

// class TlvValue;
class TlvObject;
// class BytesBuffer;

class TlvArray
{
  public:
    TlvArray();
    TlvArray(TlvArray &&a);
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
class TlvObject
{
  public:
    TlvObject();
    TlvObject(TlvObject &&o);
    ~TlvObject();

    int serialize(uint16_t tag, std::vector<uint8_t> &out) const;
    inline int serialize(std::vector<uint8_t> &out) const { return serialize(0, out); }
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

    // tag range in [0,2047]
    void insert(int tag, const TlvValue &v);
    void insert(int tag, TlvValue &&v);
    void insert(int tag, TlvObject &&v);
    void insert(int tag, TlvArray &&v);
    void insert(int tag, const std::vector<uint8_t>& v);
    void insert(int tag, const std::vector<int8_t>& v);

    TlvValue *get(int tag) const;
    TlvValue& at(int tag) const;
    bool has(int tag) const;
    size_t size() const;

  private:
    TlvObject &operator=(const TlvObject &c);

  public:
  private:
    std::map<int, TlvValue> _values;
};
inline void TlvObject::insert(int tag, const TlvValue &v) { _values[tag] = v; }
inline void TlvObject::insert(int tag, TlvValue &&v) { _values[tag] = std::move(v); }

inline void TlvObject::insert(int tag, TlvObject &&v)
{
    TlvValue val;
    val.set_ptr(new TlvObject(std::move(v)), 100);
    _values[tag] = std::move(val);
}
inline void TlvObject::insert(int tag, TlvArray &&v)
{
    TlvValue val;
    val.set_ptr(new TlvArray(std::move(v)), 101);
    _values[tag] = std::move(val);
}

inline void TlvObject::insert(int tag,const std::vector<uint8_t>& v)
{
    TlvValue val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline void TlvObject::insert(int tag, const std::vector<int8_t>& v)
{
    TlvValue val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline TlvValue *TlvObject::get(int tag) const
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return nullptr;
    }
    return &(it->second);
}

inline TlvValue& TlvObject::at(int tag) const
{
    return _values.at(tag);
}

inline bool TlvObject::has(int tag) const
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return false;
    } 
    return true;
}

inline size_t TlvObject::size() const { return _values.size(); }

///////////////////////////////////////
// array<object>
// int serialize_array0(uint16_t tag, const std::vector<TlvObject>& values, std::vector<uint8_t>& out);

int serialize_bytes(uint16_t tag, const char *values, size_t size, std::vector<uint8_t> &out);

int serialize_array2(uint16_t tag, const std::vector<int16_t> &values, std::vector<uint8_t> &out);
int serialize_array2(uint16_t tag, const std::vector<uint16_t> &values, std::vector<uint8_t> &out);
int serialize_array4(uint16_t tag, const std::vector<int32_t> &values, std::vector<uint8_t> &out);
int serialize_array4(uint16_t tag, const std::vector<uint32_t> &values, std::vector<uint8_t> &out);
int serialize_array4(uint16_t tag, const std::vector<float> &values, std::vector<uint8_t> &out);
int serialize_array8(uint16_t tag, const std::vector<int64_t> &values, std::vector<uint8_t> &out);
// int serialize_array8(uint16_t tag, const std::vector<uint64_t>& values, std::vector<uint8_t>& out);
int serialize_array8(uint16_t tag, const std::vector<double> &values, std::vector<uint8_t> &out);
int serialize_object(uint16_t tag, const std::vector<double> &values, std::vector<uint8_t> &out);

} // namespace tlv
