#pragma once
#include <tuple_record.h>
#include <tlv_types.h>
#include <tlv_value.h>
#include "mut_table_key.h"

namespace tlv
{
// 数据格式
//  key [表ID + 分区ID + 主键]
// data [TLV数据 + 时间戳]

//
class TlvFieldInfo
{
  public:
    std::string name;    // 1 字段名
    uint16_t id    = 0;  // 2 字段id
    uint16_t type  = 0;  // 3 数据类型
    uint32_t flags = 0;  // 4 扩展标记，0x01 主键 primary key, 0x02 允许 none, 0x04 unsigned
    uint32_t len   = 0;  // 5 数据长度
    std::string comment; // 6 注释
    std::string extra;   // 7 扩展字段, json or msgpack or cbor

    uint32_t size() const { return len; }
    // 数据类型, long double string
    uint8_t dtype() const { return 0x0ff & type; }
    void set_dtype(uint8_t t) { type = (type & 0xff00) | t; }
    // 展示类型 int8,int16,int32, int64,double,float,text, bytes, blob,
    // date,datetime, JSON, CBOR
    uint8_t ptype() const { return (0xff00 & type) >> 8; }
    void set_ptype(uint8_t t) { type = (type & 0x00ff) | (t << 8); }

    void set_type(uint16_t dtp, uint16_t ptp) { type = dtp | (ptp << 8); }
    bool is_unsigned() const { return flags & 0x04; }
    int serialize(TupleRecord &out) const
    {
        out.insert(1, TlvValue(name));
        out.insert(2, TlvValue(id));
        out.insert(3, TlvValue(type));
        out.insert(4, TlvValue(flags));
        out.insert(5, TlvValue(len));
        out.insert(6, TlvValue(comment));
        out.insert(7, TlvValue(extra));
        return 0;
    }
    int deserialize(const TupleRecord &in)
    {
        if (in.has(1)) name = in.get(1)->to_string();

        if (in.has(2)) id = in.get(2)->to_int();

        if (in.has(3)) type = in.get(3)->to_int();

        if (in.has(4)) flags = in.get(4)->to_int();

        if (in.has(5)) len = in.get(5)->to_int();

        if (in.has(6)) comment = in.get(6)->to_string();

        if (in.has(7)) extra = in.get(7)->to_string();
        return 0;
    }
};

// 表信息格式
//  key [表ID + 分区ID + 主键]
// data [TLV数据 + 时间戳 + 状态(正常，删除中，已删除)]
class TlvTableInfo
{
  public:
    uint32_t id;                        // 1 表id
    uint32_t version;                   // 2 版本
    std::string name;                   // 3
    std::string comment;                // 4 注释
    std::vector<uint8_t> primarykey;    // 5 主键
    std::map<int, TlvFieldInfo> fields; // 6 字段信息 <字段id，字段信息>,  字段0内部使用
    // field: 0 object
    /**
     * 1 {name: updatetime, id: 0, dtype:int64}
     * 2 {name: createtime, id: 0, dtype:int64}
     * 3 {name: modifytime, id: 0, dtype:int64}
     */

    int get_primary_key_info(std::vector<const TlvFieldInfo *> infos)
    {
        for (auto i : primarykey) {
            auto const it = fields.at(i);
            infos.push_back(&it);
        }
        return 0;
    }

    int get_index_key_info(const std::vector<int> &ids, std::vector<const TlvFieldInfo *> infos)
    {
        for (auto i : ids) {
            auto const it = fields.at(i);
            infos.push_back(&it);
        }
        return 0;
    }

    // 获取索引key
    int get_index_key(const std::vector<int> &fields,TupleRecord &obj, MutTableKey &key);
    // 获取主键key
    int get_primary_key(TupleRecord &obj, MutTableKey &key);

    int serialize(std::vector<uint8_t> &out)
    {
        TupleRecord obj;
        obj.insert(1, TlvValue(id));
        obj.insert(2, TlvValue(version));
        obj.insert(3, TlvValue(name));
        obj.insert(4, TlvValue(comment));
        obj.insert(5, TlvValue(reinterpret_cast<const char *>(primarykey.data()), primarykey.size()));
        TupleRecord subobj;
        obj.insert(6, std::move(subobj));
        for (auto const &field : fields) {
            TupleRecord item;
            field.second.serialize(item);
            subobj.insert(field.first, std::move(item));
        }
        return obj.serialize(out);
    }
    int deserialize(const std::vector<uint8_t> &in)
    {
        TupleRecord obj;
        int rc = obj.deserialize(in);
        if (rc != 0) {
            return rc;
        }

        if (obj.has(1)) id = obj.get(1)->to_int();

        if (obj.has(2)) version = obj.get(2)->to_int();

        if (obj.has(3)) name = obj.get(3)->to_string();

        if (obj.has(4)) comment = obj.get(4)->to_string();

        if (obj.has(5)) {
            auto v = obj.get(5);
            for (size_t i = 0; i < v->size(); ++i) {
                primarykey.push_back((uint8_t)(v->data()[i]));
            }
        }

        if (obj.has(6)) {
            auto subobj = obj.get(6)->as_object<TupleRecord>();
            // TODO
            TlvFieldInfo field;
            if (field.deserialize(*subobj) != 0) {
                // TODO
                return -1;
            }

            int fid = field.id;
            fields.emplace(fid, std::move(field));
        }

        return rc;
    }
};
struct TlvIndexInfo
{
    std::string name;            // 索引名
    std::string tblname;         // 表名
    std::vector<uint8_t> fields; // 索引字段
    uint8_t type;                // 索引类型
};

}; // namespace tlv