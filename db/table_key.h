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

namespace aquadb{
inline int end_key_compare(const Str& key1, const Str& key2) {
    if (key1 == key2) {
        return 0;
    }
    if (key1.empty()) {
        return 1;
    }
    if (key2.empty()) {
        return -1;
    }
    return key1.compare(key2);
}
class MutTableKey;
class TableKey {
public:
    virtual ~TableKey() {}
    TableKey() : _full(false), _data(nullptr,0) {}

    // create TableKey from a slice, use for extract fields
    TableKey(const Str& key, bool full = true) : 
        _full(full), 
        _data(key.data(), key.size()) {}

    TableKey(const char* s, size_t size, bool full = false) : _full(full), _data(s, size) {}

    TableKey(const TableKey& key) : 
        _full(key._full),
        _data(key._data.data(), key._data.size()) {}

    TableKey(const MutTableKey& key);

    //TODO
    void skip_table_prefix(int &pos) {
        pos += sizeof(int64_t);
    }

    void skip_region_prefix(int &pos) {
        pos += sizeof(int64_t);
    }

    int8_t extract_i8(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::decode_i8(*reinterpret_cast<uint8_t*>(c));
    }

    uint8_t extract_u8(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return *reinterpret_cast<uint8_t*>(c);
    }

    int16_t extract_i16(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::decode_i16(
            KeyEncoder::to_endian_u16(*reinterpret_cast<uint16_t*>(c)));
    }

    uint16_t extract_u16(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::to_endian_u16(*reinterpret_cast<uint16_t*>(c));
    }

    int32_t extract_i32(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::decode_i32(
            KeyEncoder::to_endian_u32(*reinterpret_cast<uint32_t*>(c)));
    }

    uint32_t extract_u32(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::to_endian_u32(*reinterpret_cast<uint32_t*>(c));
    }

    int64_t  extract_i64(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::decode_i64(
            KeyEncoder::to_endian_u64(*reinterpret_cast<uint64_t*>(c)));
    }

    uint64_t extract_u64(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::to_endian_u64(*reinterpret_cast<uint64_t*>(c));
    }

    float extract_float(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::decode_f32(
            KeyEncoder::to_endian_u32(*reinterpret_cast<uint32_t*>(c)));
    }

    double extract_double(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return KeyEncoder::decode_f64(
            KeyEncoder::to_endian_u64(*reinterpret_cast<uint64_t*>(c)));
    }

    bool extract_boolean(int pos) const {
        char* c = const_cast<char*>(_data.data() + pos);
        return (*reinterpret_cast<uint8_t*>(c)) != 0;
    }

    void extract_string(int pos, std::string& out) const {
        out.assign(_data.data() + pos);
        return;
    }

    void extract_char(int pos, size_t len, std::string& out) {
        out.assign(_data.data() + pos, len);
    }

    void set_full(bool full) {
        _full = full;
    }

    bool get_full() const{
        return _full;
    }

    size_t size() const {
        return _data.length();
    }

    const StrRef& data() const {
        return _data;
    }

private:
    bool             _full;  //full key or just a prefix
    StrRef          _data;

};
}

