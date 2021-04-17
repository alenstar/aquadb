#pragma once

#include <map>
#include <string>
#include <tlv_types.h>
#include <tlv_value.h>
#include <vector>

namespace tlv
{

// class TlvValue;
class TupleRecord;
// class BytesBuffer;

class TlvArray
{
  public:
    TlvArray();
    TlvArray(TlvArray &&a);
    ~TlvArray();

    void append(const TlvValue &v);
    void append(const TupleRecord &v);

    int serialize(uint16_t tag, std::vector<uint8_t> &out) const;
    int deserialize(const std::vector<uint8_t> &in) const;
    int deserialize(const std::string &in) const;
    int deserialize(BytesBuffer *in, uint16_t &tag) const;

  private:
    std::vector<TlvValue> _values;
};

// TupleRecord or TlvObj
class TupleRecord
{
  public:
    TupleRecord();
    TupleRecord(TupleRecord &&o);
    ~TupleRecord();

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
    void insert(int tag, TupleRecord &&v);
    void insert(int tag, TlvArray &&v);
    void insert(int tag, const std::vector<uint8_t>& v);
    void insert(int tag, const std::vector<int8_t>& v);

    const TlvValue *get(int tag) const;
    TlvValue *get(int tag);

    const TlvValue& at(int tag) const;
    TlvValue& at(int tag) ;

    bool has(int tag) const;
    size_t size() const;

  private:
    TupleRecord &operator=(const TupleRecord &c);

  public:
  private:
    std::map<int, TlvValue> _values;
};
inline void TupleRecord::insert(int tag, const TlvValue &v) { _values[tag] = v; }
inline void TupleRecord::insert(int tag, TlvValue &&v) { _values[tag] = std::move(v); }

inline void TupleRecord::insert(int tag, TupleRecord &&v)
{
    TlvValue val;
    val.set_ptr(new TupleRecord(std::move(v)), 100);
    _values[tag] = std::move(val);
}
inline void TupleRecord::insert(int tag, TlvArray &&v)
{
    TlvValue val;
    val.set_ptr(new TlvArray(std::move(v)), 101);
    _values[tag] = std::move(val);
}

inline void TupleRecord::insert(int tag,const std::vector<uint8_t>& v)
{
    TlvValue val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline void TupleRecord::insert(int tag, const std::vector<int8_t>& v)
{
    TlvValue val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline const TlvValue *TupleRecord::get(int tag) const
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return nullptr;
    }
    return &(it->second);
}

inline const TlvValue& TupleRecord::at(int tag) const
{
    return _values.at(tag);
}


inline TlvValue *TupleRecord::get(int tag) 
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return nullptr;
    }
    return &(it->second);
}

inline TlvValue& TupleRecord::at(int tag) 
{
    return _values.at(tag);
}

inline bool TupleRecord::has(int tag) const
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return false;
    } 
    return true;
}

inline size_t TupleRecord::size() const { return _values.size(); }

///////////////////////////////////////
// array<object>
// int serialize_array0(uint16_t tag, const std::vector<TupleRecord>& values, std::vector<uint8_t>& out);

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
