#pragma once
#include <tlv_types.h>
#include <tlv_value.h>
#include <tlv_object.h>

namespace tlv
{
    // 数据格式
    //  key [表ID + 分区ID + 主键]
    // data [TLV数据 + 时间戳]

    // 
    class TlvFieldInfo
    {
        public:
        std::string name; // 1 字段名 
        uint16_t id = 0; // 2 字段id
        uint16_t type = 0; // 3 数据类型
        uint32_t flags =0 ;// 4 扩展标记，0x01 主键 primary key, 0x02 允许 none
        uint32_t len = 0; // 5 数据长度
        std::string comment;// 6 注释
        std::string extra; // 7 扩展字段, json or msgpack or cbor

        // 数据类型, long double string
        uint8_t dtype()const { return 0x0ff & type;}
        // 展示类型 int8,int16,int32, int64,double,float,text, bytes, blob, date,datetime, JSON, CBOR
        uint8_t ptype()const { return (0xff00 & type) >> 8;}

        int serialize(TlvObject &out) const
        {
            out.insert(1, TlvValue(name));
            out.insert(2, TlvValue(id));
            out.insert(3, TlvValue(type));
            out.insert(4, TlvValue(flags));
            out.insert(5, TlvValue(len));
            out.insert(6, TlvValue(comment));
            out.insert(7, TlvValue(extra));
        }
        int deserialize(const TlvObject &in)
        {
            if(in.has(1))
            name = in.get(1)->to_string();

            if(in.has(2))
            id = in.get(2)->to_int();

            if(in.has(3))
            type = in.get(3)->to_int();

            if(in.has(4))
            flags = in.get(4)->to_int();

            if(in.has(5))
            len = in.get(5)->to_int();

            if(in.has(6))
            comment = in.get(6)->to_string();

            if(in.has(7))
            extra = in.get(7)->to_string();
        }
    };

    // 表信息格式
    //  key [表ID + 分区ID + 主键]
    // data [TLV数据 + 时间戳 + 状态(正常，删除中，已删除)]
    class TlvTableInfo
    {
        public:
        uint32_t id; // 1 表id
        uint32_t version; // 2 版本
        std::string name; // 3
        std::string comment;// 4 注释
        std::vector<uint8_t> primarykey; // 5 主键
        std::map<int, TlvFieldInfo> fields; // 6 字段信息 <字段id，字段信息>,  字段0内部使用
        // field: 0 object
        /**
         * 1 {name: updatetime, id: 0, dtype:int64}
         * 2 {name: createtime, id: 0, dtype:int64}
         * 3 {name: modifytime, id: 0, dtype:int64}
         */

        int serialize(std::vector<uint8_t> &out)
        {
            TlvObject obj;
            obj.insert(1, TlvValue(id));
            obj.insert(2, TlvValue(version));
            obj.insert(3, TlvValue(name));
            obj.insert(4, TlvValue(comment));
            obj.insert(5, TlvValue(reinterpret_cast<const char*>(primarykey.data()), primarykey.size()));
            TlvObject subobj;
            obj.insert(6, std::move(subobj));
            for(auto const& field: fields)
            {
                TlvObject item;
                field.second.serialize(item);
                subobj.insert(field.first, std::move(item));
            }
            return obj.serialize(out);
        }
        int deserialize(const std::vector<uint8_t> &in)
        {
            TlvObject obj;
            int rc = obj.deserialize(in);
            if(rc != 0)
            {
                return rc;
            }

            if(obj.has(1)) 
            id = obj.get(1)->to_int();

            if(obj.has(2)) 
            version = obj.get(2)->to_int();

            if(obj.has(3)) 
            name = obj.get(3)->to_string();

            if(obj.has(4)) 
            comment = obj.get(4)->to_string();

            if(obj.has(5))  {
            auto v = obj.get(5);
            for(size_t i = 0; i < v->size(); ++i){
                primarykey.push_back((uint8_t)(v.data() + i));
            }
            }

            if(obj.has(6))  {
            auto subobj = obj.get(6)->as_object_ref<TlvObject>();
            // TODO
            TlvFieldInfo field;
            if(field.deserialize(subobj) != 0) {
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
        std::string name;// 索引名
        std::string tblname;// 表名
        std::vector<uint8_t> fields;// 索引字段
        uint8_t type ;// 索引类型
    };

    class TlvMessage
    {
        public:
            TlvMessage();
            int parse(const std::string& in);
            // 获取索引key
            TlvValue get_index_key(const std::vector<uint8_t>& fields);
            // 获取主键key 
            TlvValue get_primary_key(const std::vector<uint8_t>& fields);

            int serialize(std::vector<uint8_t> &out)
            {
                return _obj.serialize(out);
            }
            int deserialize(const std::vector<uint8_t> &in)
            {
                return _obj.deserialize(in);
            }
        private:
        TlvObject _obj;
        std::string _raw_data; // 原始数据
        std::string _pk_data; // 主键数据
    };

};