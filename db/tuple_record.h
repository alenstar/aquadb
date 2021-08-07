#pragma once

#include <map>
#include <string>
#include "data_types.h"
#include "tuple_value.h"
#include <vector>

namespace aquadb
{

// class Value;
class TupleRecord;
// class BufferView;

class Array
{
  public:
    Array();
    Array(Array &&a);
    ~Array();

    void append(const Value &v);
    void append(const TupleRecord &v);

    int serialize(uint16_t tag, std::vector<uint8_t> &out) const;
    int deserialize(const std::vector<uint8_t> &in) ;
    int deserialize(const std::string &in) ;
    int deserialize(BufferView *in, uint16_t &tag) ;

  private:
    std::vector<Value> _values;
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
    int deserialize(BufferView& in);
    int deserialize(BufferView *in, uint16_t &tag);

    // tag range in [0,2047]
    void insert(int tag, const Value &v);
    void insert(int tag, Value &&v);
    void insert(int tag, TupleRecord &&v);
    void insert(int tag, Array &&v);
    void insert(int tag, const std::vector<uint8_t>& v);
    void insert(int tag, const std::vector<int8_t>& v);

    const Value *get(int tag) const;
    Value *get(int tag);

    const Value& at(int tag) const;
    Value& at(int tag) ;

    bool has(int tag) const;
    size_t size() const;
    void clear() ;

    const std::map<int, Value>& values() const {return _values;}
  private:
    TupleRecord &operator=(const TupleRecord &c);

  public:
  private:
    std::map<int, Value> _values;
};
inline void TupleRecord::insert(int tag, const Value &v) { _values[tag] = v; }
inline void TupleRecord::insert(int tag, Value &&v) { _values[tag] = std::move(v); }

inline void TupleRecord::insert(int tag, TupleRecord &&v)
{
    Value val;
    val.set_ptr(new TupleRecord(std::move(v)), 100);
    _values[tag] = std::move(val);
}
inline void TupleRecord::insert(int tag, Array &&v)
{
    Value val;
    val.set_ptr(new Array(std::move(v)), 101);
    _values[tag] = std::move(val);
}

inline void TupleRecord::insert(int tag,const std::vector<uint8_t>& v)
{
    Value val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline void TupleRecord::insert(int tag, const std::vector<int8_t>& v)
{
    Value val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline const Value *TupleRecord::get(int tag) const
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return nullptr;
    }
    return &(it->second);
}

inline const Value& TupleRecord::at(int tag) const
{
    return _values.at(tag);
}


inline Value *TupleRecord::get(int tag) 
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return nullptr;
    }
    return &(it->second);
}

inline Value& TupleRecord::at(int tag) 
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
inline void TupleRecord::clear() { return _values.clear(); }


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

} // namespace aquadb
