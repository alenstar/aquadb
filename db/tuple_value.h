#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "data_types.h"

#define VALUE_MAX_STRING_LENGTH (sizeof(int64_t) + sizeof(int32_t) - 1)
#define VALUE_MAX_LOCAL_LENGTH (sizeof(int64_t) + sizeof(int32_t))

namespace aquadb
{
//////////////////////////////////////////////////////////////////////////////////

class BufferArray
{
  public:
    BufferArray() {}
    BufferArray(std::vector<uint8_t> &&buf) : _data(std::move(buf)) {}
    BufferArray(BufferArray &&buf) : _data(std::move(buf._data)) {}
    BufferArray &operator=(BufferArray &&cv) noexcept
    {
        if (this == &cv)
        {
            return *this;
        }
        _data = std::move(cv._data);
    }
    inline std::vector<uint8_t> &operator()() noexcept { return _data; }
    inline void push_back(uint8_t v) { return _data.push_back(v); }
    inline size_t size() const { return _data.size(); }
    inline size_t capacity() const { return _data.capacity(); }
    inline void reserve(size_t sz) { _data.reserve(sz); }
    inline void resize(size_t sz) { _data.resize(sz); }
    inline uint8_t *data() { return _data.data(); }
    inline const uint8_t *data() const { return _data.data(); }

  private:
    std::vector<uint8_t> _data;
};

class BufferView
{
  public:
    BufferView(uint8_t *data, size_t size) : _data(data), _size(size) {}
    BufferView(const char *data, size_t size) : _data(reinterpret_cast<uint8_t*>( const_cast<char*>(data))), _size(size) {}
    BufferView(const std::vector<uint8_t> &data) : _data(const_cast<uint8_t *>(data.data())), _size(data.size()) {}
    inline uint8_t *peek(size_t n)
    {
        if (n > _size || _size == 0)
        {
            return nullptr;
        }
        _data = _data + n;
        _size = _size - n;
        return _data;
    }
    inline size_t size() const { return _size; }
    inline uint8_t *data() const { return _data; }
    inline bool empty() const { return _size == 0; }

  private:
    uint8_t *_data{nullptr};
    size_t _size{0};
};

/**
 * @brief 单元值
 *
 */
class Value
{
  public:
    enum DataType
    {
        NONE = 0,
        DOUBLE = 1,
        LONG = 2,
        STRING = 3,
        ULONG = 4,
        OBJECT = 5,
        ARRAY = 6,
    };

    /**
     * @brief 构造空对象
     *
     */
    Value();

    /**
     * @brief 使用整型构造对象
     *
     * @param v
     */
    Value(int v);

    /**
     * @brief 使用长整型构造对象
     *
     * @param v
     */
    Value(int64_t v);
    Value(uint64_t v);

    /**
     * @brief 使用无符号整型构造对象
     *
     * @param v
     */
    Value(uint32_t v);

    /**
     * @brief 使用浮点数构造对象
     *
     * @param v
     */
    Value(double v);

    /**
     * @brief 使用字符串构造对象
     *
     * @param v
     */
    Value(const std::string &v);

    Value(const char *v, size_t size);

    Value(const Value &cv);

    Value(Value &&cv);

    Value &operator=(const Value &cv);

    Value &operator=(Value &&cv) noexcept;

    ~Value();

    /**
     * @brief 清空对象
     *
     */
    void clear();

    /**
     * @brief 设置对象值
     *
     * @param v
     */
    void set_value(int v);

    void set_value(int64_t v);
    void set_value(uint64_t v);

    void set_value(uint32_t v);

    void set_value(double v);

    void set_value(const std::string &v);
    void set_value(const char *v, size_t size);

    int32_t to_i32() const;
    uint32_t to_u32() const;

    void set_ptr(void *ptr, uint8_t dtype);

    /**
     * @brief 获取整型值, 自动类型转换
     *
     * @return int64_t
     */
    int64_t to_long() const;
    // int64_t to_ulong() const;

    /**
     * @brief 获取浮点值, 自动类型转换
     *
     * @return double
     */
    double to_double() const;

    /**
     * @brief 获取字符串值, 自动类型转换
     *
     * @return std::string
     */
    std::string to_string() const;

    /**
     * @brief 获取内部字符串值，不进行类型转换
     *
     * @return const std::string&
     */
    // const std::string& get_string() const;

    const char *c_str() const;

    const char *data() const;

    size_t size() const;

    template <typename T> const T *as_object() const
    {
        if (_dtype > 4)
        {
            return reinterpret_cast<const T *>(_sval);
        }
        else
        {
            throw std::invalid_argument("invalid object pointer");
        }
    }

    template <typename T> const T as_object_ref() const
    {
        if (_dtype > 4)
        {
            return *(reinterpret_cast<const T *>(_sval));
        }
        else
        {
            throw std::invalid_argument("invalid object pointer");
        }
    }

    /**
     * @brief 获取值数据类型
     *    0: 空， 1：浮点型， 2：整型， 3: 字符串
     * @return int
     */
    int dtype() const;

    /**
     * @brief 是否为空
     *
     * @return true
     * @return false
     */
    bool is_none() const;

    /**
     * @brief 是否浮点值
     *
     * @return true
     * @return false
     */
    bool is_double() const;

    /**
     * @brief 是否整型值
     *
     * @return true
     * @return false
     */
    bool is_long() const;

    /**
     * @brief 是否字符串
     *
     * @return true
     * @return false
     */
    bool is_string() const;

    friend std::ostream &operator<<(std::ostream &oss, const Value &o);

    uint8_t wrie_type() const;

    int serialize(uint16_t tag, std::vector<uint8_t> &out) const;

    /**
     * @brief
     *
     * @param in
     * @param tag
     * @return int != 0 fail； ==0 success
     */
    // int deserialize(const std::string& in);
    int deserialize(BufferView *in, uint16_t &tag);

  private:
    union {
        char _buffer[sizeof(int64_t)];
        double _fval;
        int64_t _ival;
        // uint64_t _uval;
        char *_sval;
    };
    char _padding[sizeof(int32_t)]; // padding
    uint32_t _size : 24;            // string size range in [0, 2^24 - 1]
    uint32_t _dtype : 7;   // 0 null, 1 double, 2 int64, 3 string, 100 tlv_object, 101 tlv_array; ragne in [0,127]
    uint32_t _isowner : 1; // the string is copy, need free
};

inline void Value::clear()
{
    if (_isowner)
    {
        free(_sval);
    }
    _size = 0;
    _dtype = 0;
    _isowner = 0;
    _sval = nullptr;
}

// inline const std::string& Value::get_string() const {return _value;}
inline int Value::dtype() const { return _dtype; }
inline uint8_t Value::wrie_type() const
{
    switch (_dtype)
    {
    case 0:
        return TLV_LTYPE_NULL; // null
    case 1:
        if (_fval == 0.0)
        {
            return TLV_LTYPE_FALSE; // false
        }
        else if (_fval == 1.0)
        {
            return TLV_LTYPE_TRUE; // true
        }
        else if (std::isnan(_fval))
        {
            return TLV_LTYPE_NAN;
        }
        else if (_fval == -1.0)
        {
            return TLV_LTYPE_ONE_NEG; // -1
        }
        return TLV_LTYPE_FLOAT64;
    case 2:
        // case 4:
        if (_ival == 0)
        {
            return TLV_LTYPE_FALSE; //
        }
        else if (_ival == 1)
        {
            return TLV_LTYPE_TRUE; //
        }
        else if (_ival == -1)
        {
            return TLV_LTYPE_ONE_NEG; //
        }
        else if (_ival > 0)
        {
            return TLV_LTYPE_VARINT_POS; // +int
        }
        return TLV_LTYPE_VARINT_NEG; // -int
    case 3:
        return TLV_LTYPE_BYTES;
    case DataType::OBJECT:
        return TLV_LTYPE_OBJECT; // object
    case DataType::ARRAY:
        return TLV_LTYPE_ARRAY0; // array
    default:
        return TLV_LTYPE_ERROR; // error , unsupported data type
    }
}

inline const char *Value::c_str() const
{
    if (_isowner)
    {
        return _sval;
    }
    else
    {
        return _buffer;
    }
}

inline const char *Value::data() const { return c_str(); }

inline size_t Value::size() const
{
    if (_dtype == 3)
    {
        return _size;
    }
    else if (_dtype == 0)
    {
        return 0;
    }
    return 8;
}

inline void Value::set_value(double v)
{
    clear();
    _dtype = 1;
    _fval = v;
}

inline void Value::set_value(int v)
{
    clear();
    _dtype = 2;
    _ival = v;
}

inline void Value::set_value(int64_t v)
{
    clear();
    _dtype = 2;
    _ival = v;
}

inline void Value::set_value(uint64_t v)
{
    clear();
    _dtype = 2;
    //_uval = v;
    _ival = static_cast<int64_t>(v);
}

inline void Value::set_value(uint32_t v)
{
    clear();
    _dtype = 2;
    _ival = v;
}

inline void Value::set_ptr(void *ptr, uint8_t type)
{
    clear();
    _dtype = type;
    _sval = reinterpret_cast<char *>(ptr);
}

inline bool Value::is_none() const { return _dtype == 0; }

inline bool Value::is_double() const { return _dtype == 1; }

inline bool Value::is_long() const { return _dtype == 2; }

inline bool Value::is_string() const { return _dtype == 3; }

inline bool operator<(const Value &left, const Value &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return left.to_double() < right.to_double();
        }
        else if (left.is_long())
        {
            return left.to_long() < right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) < 0;
        }
    }
    throw std::logic_error("dtype mismatch");
}

inline bool operator>(const Value &left, const Value &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return left.to_double() > right.to_double();
        }
        else if (left.is_long())
        {
            return left.to_long() > right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) > 0;
        }
    }
    throw std::logic_error("dtype mismatch");
}

inline bool operator==(const Value &left, const Value &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return static_cast<int64_t>((left.to_double() - right.to_double()) * 1000000000000L) == 0;
        }
        else if (left.is_long())
        {
            return left.to_long() == right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) == 0;
        }
    }
    return false;
}

inline bool operator!=(const Value &left, const Value &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return static_cast<int64_t>((left.to_double() - right.to_double()) * 1000000000000L) != 0;
        }
        else if (left.is_long())
        {
            return left.to_long() != right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) != 0;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
}
