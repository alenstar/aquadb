#include "descriptor.h"


namespace aquadb
{



inline int field_fixed_size(const FieldDescriptor *field) {
    auto type = field->type; 
    switch (type) {
        case FieldDescriptor::FieldType::Int8:
        case FieldDescriptor::FieldType::UInt8:
        return 1;
        case FieldDescriptor::FieldType::Int16:
        case FieldDescriptor::FieldType::UInt16:
        return 2;
        case FieldDescriptor::FieldType::Int32:
        case FieldDescriptor::FieldType::UInt32:
        return 4;
        case FieldDescriptor::FieldType::Int64:
        case FieldDescriptor::FieldType::UInt64:
        return 8;
        case FieldDescriptor::FieldType::Float32:
        return 4;
        case FieldDescriptor::FieldType::Float64:
        return 8;
        case FieldDescriptor::FieldType::FixedString:
            return field->size();
        case FieldDescriptor::FieldType::String:
        // 变长数据返回0
        return 0;
        default:
        // 不支持的类型
        return -1;
    }
}

    int IndexDescriptor::key_size(const TableDescriptor* p) const {
        int ksz = 0;
        for (auto f: fields)
        {
            auto it = p->fields.find(f);
            if(it == p->fields.cend()) {
                return -1;
            }
            //auto type = it->second.type;
            ksz += field_fixed_size(&(it->second));
        }
        return ksz;
    }


// 获取索引key
int TableDescriptor::get_index_key(const std::vector<uint16_t> &fields, TupleObject &obj, MutTableKey &key)
{
    std::vector<const FieldDescriptor *> infos;
    int rc = get_index_key_info(fields, infos);
    if(rc != 0)
    {
        return rc;
    }
    for (auto i : infos) {
        auto v = obj.get(i->id);
        if (!v) {
            return -1;
        }
        field_append_value(i, v, key);
    }
    return 0;
}
// 获取主键key
int TableDescriptor::get_primary_key(TupleObject &obj,MutTableKey &key)
{

    std::vector<const FieldDescriptor *> infos;
    int rc = get_primary_key_info(infos);
        if(rc != 0)
    {
        return rc;
    }
    for (auto i : infos) {
        auto v = obj.get(i->id);
        if (!v) {
            return -1;
        }
        field_append_value(i, v, key);
    }
    return 0;
}

} // namespace end of tlv