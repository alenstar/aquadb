// Copyright (c) 2018-present Baidu, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "key_encoder.h"
#include "str.h"

namespace aquadb {

class TableKey;
class MutTableKey {
public:
    virtual ~MutTableKey() {}
    MutTableKey() : _full(false) {}
    MutTableKey(MutTableKey&& k) : _full(k._full), _data(std::move(k._data)) {}
    MutTableKey(const MutTableKey& k) : _full(k._full),_data(k._data) {}
    MutTableKey &operator=(MutTableKey &&cv) noexcept
    {
        if (this == &cv)
        {
            return *this;
        }
        _full = cv._full;
        _data = std::move(cv._data);
    }

    // create TableKey from a slice, use for extract fields
    MutTableKey(const Str& key, bool full = true) : 
        _full(full), 
        _data(key.data(), key.size()) {}

    MutTableKey(const TableKey& key);

    MutTableKey& append_i8(int8_t val) {
        uint8_t encode = KeyEncoder::encode_i8(val);
        _data.append((char*)&encode, sizeof(uint8_t));
        return *this;
    }

    MutTableKey& append_u8(uint8_t val) {
        _data.append((char*)&val, sizeof(uint8_t));
        return *this;
    }

    MutTableKey& replace_u8(uint8_t val, int pos) {
        _data.replace(pos, 1, (char*)&val, 1);
        return *this;
    }

    MutTableKey& append_i16(int16_t val) {
        uint16_t encode = KeyEncoder::to_endian_u16(KeyEncoder::encode_i16(val));
        _data.append((char*)&encode, sizeof(uint16_t));
        return *this;
    }

    MutTableKey& append_u16(uint16_t val) {
        uint16_t encode = KeyEncoder::to_endian_u16(val);
        _data.append((char*)&encode, sizeof(uint16_t));
        return *this;
    }

    MutTableKey& append_i32(int32_t val) {
        uint32_t encode = KeyEncoder::to_endian_u32(KeyEncoder::encode_i32(val));
        _data.append((char*)&encode, sizeof(uint32_t));
        return *this;
    }

    MutTableKey& replace_i32(int32_t val, size_t pos) { // cstore
       uint32_t encode = KeyEncoder::to_endian_u32(KeyEncoder::encode_i32(val));
       size_t len = sizeof(uint32_t);
       _data.replace(pos, len, (char*)&encode, sizeof(uint32_t));
       return *this;
    }

    MutTableKey& append_u32(uint32_t val) {
        uint32_t encode = KeyEncoder::to_endian_u32(val);
        _data.append((char*)&encode, sizeof(uint32_t));
        return *this;
    }

    MutTableKey& append_i64(int64_t val) {
        uint64_t encode = KeyEncoder::to_endian_u64(KeyEncoder::encode_i64(val));
        _data.append((char*)&encode, sizeof(uint64_t));
        return *this;
    }

    MutTableKey& replace_i64(int64_t val, size_t pos) {
        uint64_t encode = KeyEncoder::to_endian_u64(KeyEncoder::encode_i64(val));
        size_t len = sizeof(uint64_t);
        _data.replace(pos, len, (char*)&encode, sizeof(uint64_t));
        return *this;
    }

    MutTableKey& append_u64(uint64_t val) {
        uint64_t encode = KeyEncoder::to_endian_u64(val);
        _data.append((char*)&encode, sizeof(uint64_t));
        return *this;
    }

    MutTableKey& append_float(float val) {
        uint32_t encode = KeyEncoder::to_endian_u32(KeyEncoder::encode_f32(val));
        _data.append((char*)&encode, sizeof(uint32_t));
        return *this;
    }

    MutTableKey& append_double(double val) {
        uint64_t encode = KeyEncoder::to_endian_u64(KeyEncoder::encode_f64(val));
        _data.append((char*)&encode, sizeof(uint64_t));
        return *this;
    }

    MutTableKey& append_string(const std::string& val) {
        _data.append(val.data(), val.size());
        _data.append('\0');
        //_data.append(1, '\0');
        return *this;
    }

    MutTableKey& append_string_prefix(const std::string& val) {
        _data.append(val.data(), val.size());
        return *this;
    }

    MutTableKey& append_char(const char* data, size_t size) {
        _data.append(data, size);
        return *this;
    }

    MutTableKey& append_fixed_char(const char* data, size_t size, size_t fixed_size) {
        size_t sz = std::min(size, fixed_size);
        _data.append(data, sz);
        if(sz < fixed_size)
        {
            _data.append(fixed_size - sz, '\0');
        }
        return *this;
    }

    MutTableKey& append_boolean(bool val) {
        uint8_t encode = val? uint8_t(1) : uint8_t(0);
        _data.append((char*)&encode, sizeof(uint8_t));
        return *this;
    }

    MutTableKey& append_index(const TableKey& key);

    MutTableKey& append_index(const MutTableKey& key) {
        _data.append(key.data(), key.size());
        return *this;
    }
    
    MutTableKey& append_index(const std::string& key) {
        _data.append(key.data(), key.size());
        return *this;
    }


    inline void set_full(bool full) {
        _full = full;
    }

    inline bool get_full() const{
        return _full;
    }

    inline size_t size() const {
        return _data.length();
    }

    inline const char* data() const {
        return _data.data();
    }

    inline char* data() {
        return _data.data();
    }

    inline const char* c_str() const { return _data.c_str();}

private:
    bool           _full;  //full key or just a prefix
    Str            _data;

};
}

