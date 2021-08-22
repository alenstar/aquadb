#pragma once

#include <map>
#include <string>
#include "data_types.h"
#include "tuple_value.h"
#include <vector>

namespace aquadb
{

// class Value;
class TupleObject;
// class BufferView;

class Array
{
  public:
    Array();
    Array(Array &&a);
    ~Array();

    void append(const Value &v);
    void append(const TupleObject &v);

    int serialize(uint16_t tag, std::vector<uint8_t> &out) const;
    int deserialize(const std::vector<uint8_t> &in) ;
    int deserialize(const std::string &in) ;
    int deserialize(BufferView *in, uint16_t &tag) ;

  private:
    std::vector<Value> _values;
};

class TupleBuffer
{
    public:
    TupleBuffer();
    inline BufferArray &operator()() noexcept { return buffer_; }
    inline const BufferArray &operator()() const noexcept { return buffer_; }

    int append(int tag,const Value& val);
    void clear() { buffer_.clear();}

    // 序列化
    static int serialize(uint16_t tag, const Value& val, BufferArray *out);
    private:
    BufferArray buffer_;
};
class TupleView
{
    public:
    TupleView(const TupleBuffer& buffer): view_(buffer()) {}
    TupleView(const BufferArray& buffer): view_(buffer) {}
    int extract(int tag, Value& val);
    int next(int& tag, Value& val);
    // 反序列化
    static int deserialize(BufferView *in, uint16_t &tag, Value& val);
    private:
    BufferView view_;
};

// TupleObject or TlvObj
class TupleObject
{
  public:
    TupleObject();
    TupleObject(TupleObject &&o);
    ~TupleObject();

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
    void insert(int tag, TupleObject &&v);
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
    TupleObject &operator=(const TupleObject &c);

  public:
  private:
    std::map<int, Value> _values;
};
inline void TupleObject::insert(int tag, const Value &v) { _values[tag] = v; }
inline void TupleObject::insert(int tag, Value &&v) { _values[tag] = std::move(v); }

inline void TupleObject::insert(int tag, TupleObject &&v)
{
    Value val;
    val.set_ptr(new TupleObject(std::move(v)), 100);
    _values[tag] = std::move(val);
}
inline void TupleObject::insert(int tag, Array &&v)
{
    Value val;
    val.set_ptr(new Array(std::move(v)), 101);
    _values[tag] = std::move(val);
}

inline void TupleObject::insert(int tag,const std::vector<uint8_t>& v)
{
    Value val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline void TupleObject::insert(int tag, const std::vector<int8_t>& v)
{
    Value val;
    val.set_value(reinterpret_cast<const char*>(v.data()), v.size());
    _values[tag] = std::move(val);
}

inline const Value *TupleObject::get(int tag) const
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return nullptr;
    }
    return &(it->second);
}

inline const Value& TupleObject::at(int tag) const
{
    return _values.at(tag);
}


inline Value *TupleObject::get(int tag) 
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return nullptr;
    }
    return &(it->second);
}

inline Value& TupleObject::at(int tag) 
{
    return _values.at(tag);
}

inline bool TupleObject::has(int tag) const
{
    auto it = _values.find(tag);
    if (it == _values.cend())
    {
        return false;
    } 
    return true;
}

inline size_t TupleObject::size() const { return _values.size(); }
inline void TupleObject::clear() { return _values.clear(); }


///////////////////////////////////////
// array<object>
// int serialize_array0(uint16_t tag, const std::vector<TupleObject>& values, std::vector<uint8_t>& out);

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
