#include "table.h"
#include "errors.h"
#include "rocks_wrapper.h"

namespace aquadb
{

int TableOperator::put(const Value &key, const Value &val)
{
    // build key
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();
    field_append_value(&(_tbl->fields.at(pk->fields.at(0))), &key, mutkey);
    rocksdb::Slice k(mutkey.data(), mutkey.size());

    // build value
    TupleObject trecord;
    trecord.insert(1, key);
    trecord.insert(2, val);

    BufferArray ba;
    trecord.serialize(ba());
    rocksdb::Slice v(reinterpret_cast<const char *>(ba.data()), ba.size());

    // store
    auto handle = _wrapper->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = _wrapper->put(wopt, handle, k, v);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        _errmsg = status.ToString();
    }
    return ERR_WRITE_ABNORMALITY;
}

int TableOperator::get(const Value &key, Value &val)
{
    auto pk = _tbl->get_primary_key();

    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);
    field_append_value(_tbl->get_field_descriptor(pk->fields.at(0)), &key, mutkey);

    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = _wrapper->seek_for_next(_wrapper->get_data_handle(), k, false);

    if (!cursor->Valid())
    {
        // error
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }

    auto v = cursor->value();
    BufferView buf(v.data(), v.size());
    TupleObject trecord;
    trecord.deserialize(buf);

    if (trecord.has(2))
    {
        //  one field
        val = trecord.at(2);
        return 0;
    }
    return ERR_NOT_FOUND;
}

int TableOperator::scan_range(const Value& start_key, const Value& end_key, std::function<bool(const TableRow& v)> fn)
{
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();

    // build start key
    MutTableKey stkey = build_table_key(start_key);
    rocksdb::Slice stk(stkey.data(), stkey.size());

    // build end key
    MutTableKey edkey = build_table_key(end_key);
    rocksdb::Slice edk(edkey.data(), edkey.size());

    TableRow row;
    // store
    rocksdb::Slice k(stkey.data(), stkey.size());
    rocksdb::Slice endk(edkey.data(), edkey.size());
    auto cursor = _wrapper->seek_for_next(_wrapper->get_data_handle(), k, false);
    for (;;)
    {
        if (!cursor->Valid())
        {
            continue;
        }
        auto k = cursor->key();
        if (k.compare(edk) > 0)
        {
            break;
        }
        row.clear();
        auto v = cursor->value();

        Value val(v.data(), v.size());
        Value key(k.data(), k.size());

        // TODO
        if(!fn(row))
        {
            break;
        }
        cursor->Next();
    }

    return 0;
}


int TableOperator::scan_prefix(const Value& prefix, std::function<bool(const TableRow& row)> fn)
{
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();

    // build start key
    MutTableKey stkey = build_table_key(prefix);
    rocksdb::Slice stk(stkey.data(), stkey.size());

    rocksdb::Slice k(stkey.data(), stkey.size());
    auto cursor = _wrapper->seek_for_next(_wrapper->get_data_handle(), k, true);

    TableRow row;
    for (;;)
    {
        if (!cursor->Valid())
        {
            continue;
        }
        row.clear();

        auto k = cursor->key();
        auto v = cursor->value();

        // TODO
        if(!fn(row))
        {
            break;
        }
        cursor->Next();
    }

    return 0;
}


int TableOperator::get_first(TableRow& record)
{
    auto pk = _tbl->get_primary_key();

    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);
    //field_append_value(_tbl->get_field_descriptor(pk->fields.at(0)), &key, mutkey);

    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = _wrapper->seek_for_next(_wrapper->get_data_handle(), k, false);

    if (!cursor->Valid())
    {
        // error
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }

    auto v = cursor->value();
    BufferView buf(v.data(), v.size());
    TupleObject trecord;
    trecord.deserialize(buf);

        if (trecord.has(1))
    {
        auto val = trecord.at(1);
        record.append("k", val);
    }

    if (trecord.has(2))
    {
        auto val = trecord.at(2);
        record.append("v", val);
    }

    return 0;
}
int TableOperator::get_last(TableRow& record) { 
    auto pk = _tbl->get_primary_key();

    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);
    //field_append_value(_tbl->get_field_descriptor(pk->fields.at(0)), &key, mutkey);

    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = _wrapper->seek_for_prev(_wrapper->get_data_handle(), k, false);

    if (!cursor->Valid())
    {
        // error
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }

    auto v = cursor->value();
    BufferView buf(v.data(), v.size());
    TupleObject trecord;
    trecord.deserialize(buf);

        if (trecord.has(1))
    {
        auto val = trecord.at(1);
        record.append("k", val);
    }

    if (trecord.has(2))
    {
        auto val = trecord.at(2);
        record.append("v", val);
    }

    return 0; 
    }


int TableOperator::insert(const TableRow &record)
{
    // build key
    MutTableKey mutkey = build_table_key(record);
    rocksdb::Slice k(mutkey.data(), mutkey.size());

    // build value
    // size_t pos = 0;
    for (auto it : _tbl->fields)
    {
        // TODO
        // check type
    }
    BufferArray ba = build_table_value(record);
    rocksdb::Slice v(reinterpret_cast<const char *>(ba.data()), ba.size());

    // store
    auto handle = _wrapper->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = _wrapper->put(wopt, handle, k, v);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        _errmsg = status.ToString();
    }
    return ERR_WRITE_ABNORMALITY;
}

int TableOperator::insert(const TableRowSet &records)
{
    auto handle = _wrapper->get_data_handle();
    auto num = records.rows_num();
    rocksdb::WriteBatch wbat;
    for (size_t i = 0; i < num; ++i)
    {
        // build key
        MutTableKey mutkey = build_table_key(records, static_cast<int>(i));
        rocksdb::Slice k(mutkey.data(), mutkey.size());

        // build value
        // size_t pos = 0;
        for (auto it : _tbl->fields)
        {
            // TODO
            // check type
        }
        BufferArray ba = build_table_value(records, static_cast<int>(i));
        rocksdb::Slice v(reinterpret_cast<const char *>(ba.data()), ba.size());
        wbat.Put(handle, k, v);
    }

    // store
    rocksdb::WriteOptions wopt;
    auto status = _wrapper->write(wopt, &wbat);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        _errmsg = status.ToString();
    }
    return ERR_WRITE_ABNORMALITY;
}

int TableOperator::remove(const Value &key)
{
    // build key
    MutTableKey mutkey = build_table_key(key);
    rocksdb::Slice k(mutkey.data(), mutkey.size());

    // store
    auto handle = _wrapper->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = _wrapper->remove(wopt, handle, k);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        _errmsg = status.ToString();
    }
    return ERR_WRITE_ABNORMALITY;
}

int TableOperator::remove_range(const Value &start_key, const Value &end_key)
{
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();

    // build start key
    MutTableKey stkey = build_table_key(start_key);
    stkey.append_u32(_db->id);
    stkey.append_u32(_tbl->id);
    field_append_value(&(_tbl->fields.at(pk->fields.at(0))), &start_key, stkey);
    rocksdb::Slice stk(stkey.data(), stkey.size());

    // build end key
    MutTableKey edkey = build_table_key(end_key);
    edkey.append_u32(_db->id);
    edkey.append_u32(_tbl->id);
    field_append_value(&(_tbl->fields.at(pk->fields.at(0))), &end_key, edkey);
    rocksdb::Slice edk(edkey.data(), edkey.size());

    // store
    auto handle = _wrapper->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = _wrapper->remove_range(wopt, handle, stk, edk, false);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        _errmsg = status.ToString();
    }
    return ERR_WRITE_ABNORMALITY;
}

int TableOperator::remove(const std::vector<Value> &row)
{
    // build key
    std::vector<MutTableKey> keys;
    for(auto const& v: row)
    {
        keys.emplace_back(build_table_key(v));
    }

    // store
    auto handle = _wrapper->get_data_handle();
    rocksdb::WriteOptions wopt;
    rocksdb::WriteBatch batch;
    for(auto& v: keys)
    {
    rocksdb::Slice k(v.data(), v.size());
        batch.Delete(handle, k);
    }
    auto status = _wrapper->write(wopt, &batch);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        _errmsg = status.ToString();
    }
    return ERR_WRITE_ABNORMALITY;
}


    MutTableKey TableOperator::build_table_key(const Value& value)
    {
    // build start key
    MutTableKey key;
    key.append_u32(_db->id);
    key.append_u32(_tbl->id);
    //key.append_string(value.data(), key.size());
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();
    field_append_value(&(_tbl->fields.at(pk->fields.at(0))), &value, key);
    return key;
    }
    MutTableKey TableOperator::build_table_key_raw(const Value& value)
    {
    // build start key
    MutTableKey key;
    key.append_u32(_db->id);
    key.append_u32(_tbl->id);
    key.append_string(value.data(), key.size());
    return key;
    }


    MutTableKey TableOperator::build_table_key(const std::vector<Value>& values)
    {
    // build start key
    MutTableKey key;
    key.append_u32(_db->id);
    key.append_u32(_tbl->id);

    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();
    for (size_t i = 0; i < pk_num; ++i)
    {
        field_append_value(&(_tbl->fields.at(pk->fields.at(i))), &(values.at(i)), key);
    }
    return key;
    }


    MutTableKey TableOperator::build_table_key(const TableRow& row) 
    {
    // build start key
    MutTableKey key;
    key.append_u32(_db->id);
    key.append_u32(_tbl->id);
    // TODO
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();
    for (size_t i = 0; i < pk_num; ++i)
    {
        auto const& field = _tbl->fields.at(pk->fields.at(i));
        auto val = row.value(field.name);
        field_append_value(&field, &val, key);
    }
return key;
    }
    MutTableKey TableOperator::build_table_key(const TableRowSet& rows, size_t rowid)
    {
    // build start key
    MutTableKey key;

    key.append_u32(_db->id);
    key.append_u32(_tbl->id);
    // TODO
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();
    for (size_t i = 0; i < pk_num; ++i)
    {
        auto const& field = _tbl->fields.at(pk->fields.at(i));
        auto val = rows.value(rowid,field.name);
        field_append_value(&field, &val, key);
    }
    return key;
    }



    BufferArray TableOperator::build_table_value(const TableRow& row)
    {
        BufferArray ba;
        TupleBuffer buffer;
        size_t size = row.size();
        for(size_t i = 0;i < size; ++i)
        {
            auto const& name = row.name(i);
            auto field = _tbl->get_field_descriptor(name);
            buffer.append(field->id, row.value(i));
        }

        ba = std::move(buffer());
        return ba;
    }

    BufferArray TableOperator::build_table_value(const TableRowSet& rows, size_t rowid)
    {
        BufferArray ba;
        TupleBuffer buffer;
        size_t size = rows.rows_num();
        for(size_t i = 0;i < size; ++i)
        {
            auto const& name = rows.name(i);
            auto field = _tbl->get_field_descriptor(name);
            buffer.append(field->id, rows.value(rowid,i));
        }

        ba = std::move(buffer());
        return ba;;
    }
//////////////////////////////////////////////////////////////////////////////


int TableReader::seek_to_first(const IndexDescriptor *index, const std::vector<Value> &key)
{
    if (key.size() > index->fields.size())
    {
        _errmsg = "Index description and value do not match";
        return ERR_INVALID_PARAMS;
    }
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);

    size_t sz = key.size();
    for (size_t i = 0; i < sz; ++i)
    {
        field_append_value(_tbl->get_field_descriptor(index->fields.at(i)), &(key.at(i)), mutkey);
    }

    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = _wrapper->seek_for_next(_wrapper->get_data_handle(), k, true);
    _iter = std::move(cursor);
    _mutkey = std::move(mutkey);

    _names.clear();
    _values.clear();
    return 0;
}

int TableReader::seek_to_last(const IndexDescriptor *index, const std::vector<Value> &key)
{
    if (key.size() > index->fields.size())
    {
        _errmsg = "Index description and value do not match";
        return ERR_INVALID_PARAMS;
    }
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);

    size_t sz = key.size();
    for (size_t i = 0; i < sz; ++i)
    {
        field_append_value(_tbl->get_field_descriptor(index->fields.at(i)), &(key.at(i)), mutkey);
    }

    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = _wrapper->seek_for_prev(_wrapper->get_data_handle(), k, true);
    _iter = std::move(cursor);
    _mutkey = std::move(mutkey);

    _names.clear();
    _values.clear();
    return 0;
}

int TableReader::seek_to(const IndexDescriptor *index, const std::vector<Value> &key, bool prefix)
{
    if (key.size() > index->fields.size())
    {
        _errmsg = "Index description and value do not match";
        return ERR_INVALID_PARAMS;
    }
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);

    size_t sz = key.size();
    for (size_t i = 0; i < sz; ++i)
    {
        field_append_value(_tbl->get_field_descriptor(index->fields.at(i)), &(key.at(i)), mutkey);
    }

    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = _wrapper->seek_for_next(_wrapper->get_data_handle(), k, prefix);
    _iter = std::move(cursor);
    _mutkey = std::move(mutkey);

    _names.clear();
    _values.clear();
    return 0;
}

int TableReader::next()
{
    if (_iter == nullptr)
    {
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }

    rocksdb::Slice prefix(_mutkey.data(), _mutkey.size());
    // for (; _iter->Valid(); _iter->Next())
    if (!_iter->Valid())
    {
        // error
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }
    {
        _iter->Next();
        if (!_iter->key().starts_with(prefix))
        {
            // finished
            _errmsg = "Data end";
            return ERR_EOF;
        }
        // TODO
        TableDescriptorPtr tbdesc = std::make_shared<TableDescriptor>();
        auto v = _iter->value();
        BufferView buf(v.data(), v.size());
        TupleObject trecord;
        trecord.deserialize(buf);

        bool firstdo = (_names.size() == 0);
        _values.clear();

        for (auto it : _tbl->fields)
        {
            if (firstdo)
            {
                _names.push_back(it.second.name);
            }
            if (trecord.has(it.first))
            {
                _values.push_back(trecord.at(it.first));
            }
            else
            {
                // TODO for default value
                Value v;
                field_default_value(&(it.second), &v);
                _values.emplace_back(std::move(v));
            }
        }
    }
    return 0;
}

int TableReader::prev()
{

    if (_iter == nullptr)
    {
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }

    rocksdb::Slice prefix(_mutkey.data(), _mutkey.size());
    // for (; _iter->Valid(); _iter->Next())
    if (!_iter->Valid())
    {
        // error
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }
    {
        _iter->Prev();
        if (!_iter->key().starts_with(prefix))
        {
            // finished
            _errmsg = "Data end";
            return ERR_EOF;
        }
        // TODO
        TableDescriptorPtr tbdesc = std::make_shared<TableDescriptor>();
        auto v = _iter->value();
        BufferView buf(v.data(), v.size());
        _record.clear();
        _record.deserialize(buf);

        bool firstdo = (_names.size() == 0);
        _values.clear();

        for (auto it : _tbl->fields)
        {
            if (firstdo)
            {
                _names.push_back(it.second.name);
            }
            if (_record.has(it.first))
            {
                _values.emplace_back(std::move(_record.at(it.first)));
            }
            else
            {
                // TODO for default value
                Value v;
                field_default_value(&(it.second), &v);
                _values.emplace_back(std::move(v));
            }
        }
    }
    return 0;
}

int TableReader::get(const std::vector<Value> &pk, TupleObject &record)
{
    if (pk.size() != _tbl->get_primary_key()->fields.size())
    {
        _errmsg = "Index description and value do not match";
        return ERR_INVALID_PARAMS;
    }
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);

    size_t sz = pk.size();
    for (size_t i = 0; i < sz; ++i)
    {
        field_append_value(_tbl->get_field_descriptor(_tbl->get_primary_key()->fields.at(i)), &(pk.at(i)), mutkey);
    }

    std::string value;
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    rocksdb::ReadOptions ropt;
    auto status = _wrapper->get(ropt, _wrapper->get_data_handle(), k, &value);
    if (!status.ok())
    {
        // error
        _errmsg = status.ToString();
        return ERR_NOT_FOUND;
    }

    TableDescriptorPtr tbdesc = std::make_shared<TableDescriptor>();
    BufferView buf(value.data(), value.size());
    record.deserialize(buf);

    return 0;
}
int TableReader::get(const std::vector<Value> &pk, std::vector<Value> &row)
{
    TupleObject record;
    int rc = get(pk, record);
    if (rc != 0)
    {
        return rc;
    }
    // TODO
    for (auto it : _tbl->fields)
    {
        if (record.has(it.first))
        {
            row.emplace_back(std::move(record.at(it.first)));
        }
        else
        {
            // TODO for default value
            Value v;
            field_default_value(&(it.second), &v);
            row.emplace_back(std::move(v));
        }
    }
    return rc;
}

int TableReader::get(const Value &key, Value &val)
{
    auto pk = _tbl->get_primary_key();
    int rc = seek_to(pk, {key});
    if (rc != 0)
    {
        return rc;
    }

    if (!_iter->Valid())
    {
        // error
        _errmsg = "Data iterator invalid";
        return ERR_INVALID_PARAMS;
    }

    auto v = _iter->value();
    BufferView buf(v.data(), v.size());
    TupleObject trecord;
    trecord.deserialize(buf);

    for (auto it : _tbl->fields)
    {
        if (it.second.flags & static_cast<uint16_t>(FieldDescriptor::FieldFlag::PrimaryKey))
        {
            continue;
        }
        if (trecord.has(it.first))
        {
            //  one field
            val = trecord.at(it.first);
            return 0;
        }
    }

    return -1;
}
}