#include "table_info.h"


namespace aquadb
{

inline int append_value(const FieldDescriptor *field, const Value *v, MutTableKey &key)
{
    auto type = static_cast<MysqlType>(field->ptype());
    switch (type) {
        case MysqlType::MYSQL_TYPE_TINY:
            key.append_i8(static_cast<int8_t>(v->to_int()));
            break;
        case MysqlType::MYSQL_TYPE_SHORT:
            key.append_i16(static_cast<int16_t>(v->to_long()));
            break;
        case MysqlType::MYSQL_TYPE_INT24:
            key.append_i32(static_cast<int32_t>(v->to_long()));
            break;
        case MysqlType::MYSQL_TYPE_LONG:
            key.append_i64(static_cast<int64_t>(v->to_long()));
            break;
        // case MysqlType::MYSQL_TYPE_LONGLONG:
        //    break;
        case MysqlType::MYSQL_TYPE_FLOAT:
            key.append_float(static_cast<float>(v->to_double()));
            break;
        case MysqlType::MYSQL_TYPE_DOUBLE:
            key.append_double(v->to_double());
            break;

        case MysqlType::MYSQL_TYPE_VARCHAR:
            // fixed char
            key.append_fixed_char(v->c_str(), v->size(), field->size());
            break;

        case MysqlType::MYSQL_TYPE_BLOB:
        case MysqlType::MYSQL_TYPE_VAR_STRING:
        case MysqlType::MYSQL_TYPE_STRING:
            // not support
            break;

        default:
            break;
    }
    return 0;
}
// 获取索引key
int TableDescriptor::get_index_key(const std::vector<int> &fields, TupleRecord &obj, MutTableKey &key)
{
    std::vector<const FieldDescriptor *> infos;
    int rc = get_index_key_info(fields, infos);
    for (auto i : infos) {
        auto v = obj.get(i->id);
        if (!v) {
            return -1;
        }
        append_value(i, v, key);
    }

    return 0;
}
// 获取主键key
int TableDescriptor::get_primary_key(TupleRecord &obj,MutTableKey &key)
{

    std::vector<const FieldDescriptor *> infos;
    int rc = get_primary_key_info(infos);
    for (auto i : infos) {
        auto v = obj.get(i->id);
        if (!v) {
            return -1;
        }
        append_value(i, v, key);
    }
    return 0;
}

} // namespace end of tlv