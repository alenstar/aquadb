#pragma once

#include "tuple_record.h"
#include "data_types.h"
#include "tuple_value.h"
#include "mut_table_key.h"


namespace aquadb
{

const uint32_t kMetaDatabaseId = 1001;
const uint32_t kMetaTableId = 1002;


class TableDescriptor;


// 数据格式
//  key [表ID + 分区ID + 主键]
// data [TLV数据 + 时间戳]

class TupleDescriptor
{
    public:
        uint16_t id; // 1 tag-id
        uint16_t type = 0; // 2 data type
        std::string name ; // 3 tag-name
        //std::string defval; // 4  default value 

    int serialize(std::vector<uint8_t> &out)
    {
        TupleRecord obj;
        obj.insert(1, Value(id));
        obj.insert(2, Value(type));
        obj.insert(3, Value(name));
        //obj.insert(4, Value(comment));
        //obj.insert(4, Value(reinterpret_cast<const char *>(fields.data()), fields.size()));
        return obj.serialize(out);
    }
    int deserialize(const std::vector<uint8_t> &in)
    {
        TupleRecord obj;
        int rc = obj.deserialize(in);
        if (rc != 0) {
            return rc;
        }

        if (obj.has(1)) id = obj.get(1)->to_i32();

        if (obj.has(2)) type = obj.get(2)->to_i32();

        if (obj.has(3)) name = obj.get(3)->to_string();

        //if (obj.has(4)) comment = obj.get(4)->to_string();

        //if (obj.has(4)) {
        //    auto v = obj.get(4);
        //    for (size_t i = 0; i < v->size(); ++i) {
        //        fields.push_back((uint8_t)(v->data()[i]));
        //    }
        //}
        return rc;
    }
};

class IndexDescriptor
{
    public:
    uint16_t id    = 0;  // 1 索引id
    uint16_t type = 0;   // 2 索引类型: 0x01 主键索引，0x02 唯一索引, 0x04 二级索引
    std::vector<uint16_t> fields; // 3 索引字段, 索引字段ID必须在1-255
    std::string name;    // 4 索引名

    inline bool is_primary_key() const { return 0x01 == type;}

    int key_size(const TableDescriptor* p) const;

    int serialize(std::vector<uint8_t> &out)
    {
        TupleRecord obj;
        obj.insert(1, Value(id));
        obj.insert(2, Value(type));
        obj.insert(3, Value(name));
        //obj.insert(4, Value(comment));
        obj.insert(4, Value(reinterpret_cast<const char *>(fields.data()), fields.size()));
        return obj.serialize(out);
    }
    int deserialize(const std::vector<uint8_t> &in)
    {
        TupleRecord obj;
        int rc = obj.deserialize(in);
        if (rc != 0) {
            return rc;
        }

        if (obj.has(1)) id = obj.get(1)->to_i32();

        if (obj.has(2)) type = obj.get(2)->to_i32();

        if (obj.has(3)) name = obj.get(3)->to_string();

        //if (obj.has(4)) comment = obj.get(4)->to_string();

        if (obj.has(4)) {
            auto v = obj.get(4);
            for (size_t i = 0; i < v->size(); ++i) {
                fields.push_back((uint8_t)(v->data()[i]));
            }
        }
        return rc;
    }
};
//
class FieldDescriptor
{
  public:
    enum class FieldFlag:uint16_t {
        None = 0,
        PrimaryKey = 0x01,
        NullAble = 0x02
    };    
    enum class FieldType:uint16_t {
        None = 0,
        Float64 = 1,
        Float32 = 2,
        Int64 = 3,
        Int32 = 4,
        Int16 = 5,
        Int8 = 6,
        FixedString = 7,
        String = 8,
        DateTime = 9
    };

    std::string name;    // 1 字段名
    uint16_t id    = 0;  // 2 字段id
    FieldType type  = FieldType::None;  // 3 数据类型
    uint16_t flags = 0;  // 4 扩展标记，0x01 主键 primary key, 0x02 允许 null, 0x04 unsigned
    union 
    {
    uint32_t len   = 0;  // 5 数据长度
    uint32_t precision; // 5 精度
    };
    std::string comment; // 6 注释
    std::string extra;   // 7 扩展字段, json or msgpack or cbor

    uint32_t size() const { return len; }
    // 数据类型, long double string
    // uint8_t dtype() const { return 0x0ff & type; }
    //void set_dtype(uint8_t t) { type = (type & 0xff00) | t; }
    // 展示类型 int8,int16,int32, int64,double,float,text, bytes, blob,
    // date,datetime, JSON, CBOR
    FieldType field_type() const { return type; }
    //void set_ptype(uint8_t t) { type = (type & 0x00ff) | (t << 8); }

    //void set_type(uint16_t dtp, uint16_t ptp) { type = dtp | (ptp << 8); }
    bool is_unsigned() const { return flags & 0x04; }
    int serialize(TupleRecord &out) const
    {
        out.insert(1, Value(name));
        out.insert(2, Value(id));
        out.insert(3, Value(static_cast<int>(type)));
        out.insert(4, Value(static_cast<int>(flags)));
        out.insert(5, Value(len));
        out.insert(6, Value(comment));
        out.insert(7, Value(extra));
        return 0;
    }
    int deserialize(const TupleRecord &in)
    {
        if (in.has(1)) name = in.get(1)->to_string();

        if (in.has(2)) id = in.get(2)->to_i32();

        if (in.has(3)) type = static_cast<FieldType>(in.get(3)->to_i32());

        if (in.has(4)) flags = static_cast<uint16_t>(in.get(4)->to_i32());

        if (in.has(5)) len = in.get(5)->to_i32();

        if (in.has(6)) comment = in.get(6)->to_string();

        if (in.has(7)) extra = in.get(7)->to_string();
        return 0;
    }
};

// 表信息格式
//  key [表ID + 分区ID + 主键]
// data [TLV数据 + 时间戳 + 状态(正常，删除中，已删除)]
class TableDescriptor
{
  public:

    uint32_t id;                        // 1 表id
    uint32_t version;                   // 2 版本
    std::string name;                   // 3 表名
    std::string comment;                // 4 注释
    //std::vector<uint16_t> primarykey;  // 5 主键, 字段ID必须在1-255
    //std::vector<IndexDescriptor*> indexs;  // 5 索引, 字段ID必须在1-255
    std::map<int, FieldDescriptor> fields; // 6 字段信息 <字段id，字段信息>,  字段0内部使用
    // field: 0 object
    /**
     * 1 {name: updatetime, id: 0, dtype:int64}
     * 2 {name: createtime, id: 0, dtype:int64}
     * 3 {name: modifytime, id: 0, dtype:int64}
     */
    uint16_t last_field_id = 0; // 7 最新字段id

    int set_primary_key(std::vector<uint16_t> ids)
    {
        auto pk = new IndexDescriptor;
        pk->fields = ids;
        pk->type = 1; // 主键索引
        if(_primarykey)
        {
            delete _primarykey;
        }
        _primarykey = pk;
        return 0;
    }
    IndexDescriptor* get_primary_key()
    {
        return _primarykey;
    }

    int get_primary_key_info(std::vector<const FieldDescriptor *> infos)
    {
        for (auto i : _primarykey->fields) {
            auto const it = fields.at(i);
            infos.push_back(&it);
        }
        return 0;
    }

    int get_index_key_info(const std::vector<uint16_t> &ids, std::vector<const FieldDescriptor *> infos)
    {
        for (auto i : ids) {
            auto const it = fields.at(i);
            infos.push_back(&it);
        }
        return 0;
    }

    // 获取索引key
    inline int get_index_key(const IndexDescriptor* index,TupleRecord &obj, MutTableKey &key) { return get_index_key(index->fields,obj, key);}
    int get_index_key(const std::vector<uint16_t> &fields,TupleRecord &obj, MutTableKey &key);
    // 获取主键key
    int get_primary_key(TupleRecord &obj, MutTableKey &key);

    int serialize(std::vector<uint8_t> &out) const
    {
        TupleRecord obj;
        obj.insert(1, Value(id));
        obj.insert(2, Value(version));
        obj.insert(3, Value(name));
        obj.insert(4, Value(comment));
        // 采用小端
        obj.insert(5, Value(reinterpret_cast<const char *>(_primarykey->fields.data()), _primarykey->fields.size() * sizeof(uint16_t)));
        TupleRecord subobj;
        obj.insert(6, std::move(subobj));
        for (auto const &field : fields) {
            TupleRecord item;
            field.second.serialize(item);
            subobj.insert(field.first, std::move(item));
        }
        obj.insert(7, Value(last_field_id));
        return obj.serialize(out);
    }
    int deserialize(const std::vector<uint8_t> &in)
    {
        BufferView bv(in);
        return deserialize(bv);
    }
    int deserialize(BufferView &in)
    {
        TupleRecord obj;
        int rc = obj.deserialize(in);
        if (rc != 0) {
            return rc;
        }

        if (obj.has(1)) id = obj.get(1)->to_i32();

        if (obj.has(2)) version = obj.get(2)->to_i32();

        if (obj.has(3)) name = obj.get(3)->to_string();

        if (obj.has(4)) comment = obj.get(4)->to_string();

        if (obj.has(5)) {
            auto v = obj.get(5);
            // 采用小端
            //const uint16_t* p = reinterpret_cast<const uint16_t*>(v->data());
            size_t sz = v->size() / sizeof(uint16_t);
            if (_primarykey)
            {
                delete _primarykey;
            }
            _primarykey = new IndexDescriptor;
            for (size_t i = 0; i < sz; ++i) {
                _primarykey->fields.push_back((uint16_t)(v->data()[i]));
            }
        }

        if (obj.has(6)) {
            auto subobj = obj.get(6)->as_object<TupleRecord>();
            // TODO
            FieldDescriptor field;
            if (field.deserialize(*subobj) != 0) {
                // TODO
                return -1;
            }

            int fid = field.id;
            fields.emplace(fid, std::move(field));
        }

        if (obj.has(7)) last_field_id = static_cast<uint32_t>(obj.get(1)->to_i32());
        return rc;
    }

    MutTableKey get_key(uint32_t metaId, uint32_t dbid) const {
        MutTableKey k;
        k.append_u32(metaId); 
        k.append_u32(dbid); 
        k.append_u32(id); 
        return k;
    }
    BufferArray get_val() const {
        BufferArray ba;
        serialize(ba());
        return ba;
    }

    const IndexDescriptor* get_index_descriptor(const std::string& name) const
    {
        auto it = std::find_if(_indexs.cbegin(), _indexs.cend(), [&name](const IndexDescriptor* index){ return index->name == name;});
        if(it == _indexs.cend())
        {
            return nullptr;
        }
        return *it;
    }
    const IndexDescriptor* get_index_descriptor(int id) const
    {
        auto it = std::find_if(_indexs.cbegin(), _indexs.cend(), [id](const IndexDescriptor* index){ return static_cast<int>(index->id) == id;});
        if(it == _indexs.cend())
        {
            return nullptr;
        }
        return *it;
    }

    const FieldDescriptor* get_field_descriptor(const std::string& name) const {
        auto it = std::find_if(fields.cbegin(), fields.cend(), [name](const std::map<int, FieldDescriptor>::value_type & item ){ return item.second.name == name;});
        if(it == fields.cend())
        {
            return nullptr;
        }
        return &(it->second);
    } 
    const FieldDescriptor* get_field_descriptor(int id) const {
        auto it = fields.find(id);
        if(it == fields.cend())
        {
            return nullptr;
        }
        return &(it->second);
    } 

    void add_field(int id, FieldDescriptor p)
    {
        fields[id] = p;
    }
    uint16_t next_id() { return ++last_field_id;}
    private:
    std::string _dbname; // 库名
    int status = 0; // 表状态, 加锁时使用
    std::vector<IndexDescriptor*> _indexs; // 索引
    IndexDescriptor* _primarykey {nullptr}; // 索引
};
typedef std::shared_ptr<TableDescriptor> TableDescriptorPtr;

// 表信息格式
//  key [metaId + 库ID]
// data [TLV数据 + 时间戳 + 状态(正常，删除中，已删除)]
class DatabaseDescriptor
{
  public:

    uint32_t id = 0;                        // 1 库id
    uint32_t version = 0;                   // 2 版本
    std::string name;                   // 3 库名
    std::string comment;                // 4 注释
    uint32_t status = 0; // 5 库状态, 加锁时使用
    std::map<int, FieldDescriptor> fields; // 6 字段信息 <字段id，字段信息>,  字段0内部使用
    // field: 0 object
    /**
     * 1 {name: updatetime, id: 0, dtype:int64}
     * 2 {name: createtime, id: 0, dtype:int64}
     * 3 {name: modifytime, id: 0, dtype:int64}
     */
    uint32_t last_tid = 0; // 7 最后的表id

    MutTableKey get_key(uint32_t metaId) const {
        MutTableKey k;
        k.append_u32(metaId); 
        k.append_u32(id); 
        return k;
    }
    BufferArray get_val() const {
        BufferArray ba;
        serialize(ba());
        return ba;
    }

    int serialize(std::vector<uint8_t> &out) const
    {
        TupleRecord obj;
        obj.insert(1, Value(id));
        obj.insert(2, Value(version));
        obj.insert(3, Value(name));
        obj.insert(4, Value(comment));
        obj.insert(5, Value(status));
        //// 采用小端
        //obj.insert(5, Value(reinterpret_cast<const char *>(_primarykey->fields.data()), _primarykey->fields.size() * sizeof(uint16_t)));
        TupleRecord subobj;
        obj.insert(6, std::move(subobj));
        for (auto const &field : fields) {
            TupleRecord item;
            field.second.serialize(item);
            subobj.insert(field.first, std::move(item));
        }
        obj.insert(7, Value(last_tid));
        return obj.serialize(out);
    }
    int deserialize(const std::vector<uint8_t> &in)
    {
        BufferView bv(in);
        return deserialize(bv);
    }
    int deserialize(BufferView &in)
    {
        TupleRecord obj;
        int rc = obj.deserialize(in);
        if (rc != 0) {
            return rc;
        }

        if (obj.has(1)) id = obj.get(1)->to_i32();

        if (obj.has(2)) version = obj.get(2)->to_i32();

        if (obj.has(3)) name = obj.get(3)->to_string();

        if (obj.has(4)) comment = obj.get(4)->to_string();

        if (obj.has(5)) {
            status = obj.get(5)->to_u32();
        }

        if (obj.has(6)) {
            auto subobj = obj.get(6)->as_object<TupleRecord>();
            // TODO
            FieldDescriptor field;
            if (field.deserialize(*subobj) != 0) {
                // TODO
                return -1;
            }

            int fid = field.id;
            fields.emplace(fid, std::move(field));
        }
        if (obj.has(7)) {
            last_tid = obj.get(7)->to_u32();
        }

        return rc;
    }

    const FieldDescriptor* get_field_descriptor(const std::string& name) const {
        auto it = std::find_if(fields.cbegin(), fields.cend(), [name](const std::map<int, FieldDescriptor>::value_type & item ){ return item.second.name == name;});
        if(it == fields.cend())
        {
            return nullptr;
        }
        return &(it->second);
    } 
    const FieldDescriptor* get_field_descriptor(int id) const {
        auto it = fields.find(id);
        if(it == fields.cend())
        {
            return nullptr;
        }
        return &(it->second);
    } 

    bool exists(const std::string& name) const {
        auto it = _name2table.find(name);
        if(it != _name2table.cend())
        {
            return true;
        }
        return false;
    }
    TableDescriptorPtr get_table(const std::string& name) {
        auto it = _name2table.find(name);
        if(it != _name2table.cend())
        {
            return it->second;
        }
        return nullptr;
    }
    void add_table(const std::string& name, TableDescriptorPtr p) {
        _name2table[name] = p;
    }

    uint32_t next_id() { return (++last_tid);}
    private:
    std::map<std::string, TableDescriptorPtr> _name2table;
};
typedef std::shared_ptr<DatabaseDescriptor> DatabaseDescriptorPtr;

    
    
inline int field_append_value(const FieldDescriptor *field, const Value *v, MutTableKey &key)
{
    auto type = field->type; // static_cast<MysqlType>(field->ptype());
    switch (type) {
        case FieldDescriptor::FieldType::Int8:
            key.append_i8(static_cast<int8_t>(v->to_i32()));
            break;
        case FieldDescriptor::FieldType::Int16:
            key.append_i16(static_cast<int16_t>(v->to_long()));
            break;
        case FieldDescriptor::FieldType::Int32:
            key.append_i32(static_cast<int32_t>(v->to_long()));
            break;
        case FieldDescriptor::FieldType::Int64:
            key.append_i64(static_cast<int64_t>(v->to_long()));
            break;
        case FieldDescriptor::FieldType::Float32:
            key.append_float(static_cast<float>(v->to_double()));
            break;
        case FieldDescriptor::FieldType::Float64:
            key.append_double(v->to_double());
            break;

        case FieldDescriptor::FieldType::FixedString:
            // fixed char
            key.append_fixed_char(v->c_str(), v->size(), field->size());
            break;

        case FieldDescriptor::FieldType::String:
            // not support
            break;

        default:
            break;
    }
    return 0;
}



inline int field_default_value(const FieldDescriptor *field, Value *v)
{
    auto type = field->type; // static_cast<MysqlType>(field->ptype());
    switch (type) {
        case FieldDescriptor::FieldType::Int8:
            v->set_value(0);
            break;
        case FieldDescriptor::FieldType::Int16:
            v->set_value(0);
            break;
        case FieldDescriptor::FieldType::Int32:
            v->set_value(0);
            break;
        case FieldDescriptor::FieldType::Int64:
            v->set_value(0);
            break;
        case FieldDescriptor::FieldType::Float32:
            v->set_value(0.0);
            break;
        case FieldDescriptor::FieldType::Float64:
            v->set_value(0.0);
            break;

        case FieldDescriptor::FieldType::FixedString:
            // fixed char
            v->set_value("");
            break;

        case FieldDescriptor::FieldType::String:
            // not support
            v->set_value("");
            break;

        default:
            break;
    }
    return 0;
}



}