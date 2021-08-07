#include "table.h"
#include "errors.h"
#include "rocks_wrapper.h"

namespace aquadb
{

int TableWriter::insert(const std::vector<Value> &row)
{
    // build key
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();
    for (size_t i = 0; i < pk_num; ++i)
    {
        field_append_value(&(_tbl->fields.at(pk->fields.at(i))), &(row.at(i)), mutkey);
    }
    rocksdb::Slice k(mutkey.data(), mutkey.size());

    // build value
    TupleRecord trecord;
    size_t pos = 0;
    for (auto it : _tbl->fields)
    {
        // TODO
        // check type
        trecord.insert(it.first, row.at(pos));
        pos++;
    }
    BufferArray ba;
    trecord.serialize(ba());
    rocksdb::Slice v(reinterpret_cast<const char *>(ba.data()), ba.size());

    // store
    auto ptr = RocksWrapper::get_instance();
    auto handle = ptr->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = ptr->put(wopt, handle, k, v);
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

int TableWriter::insert(const TupleRecord &record)
{
    // build key
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);
    auto pk = _tbl->get_primary_key();
    //size_t pk_num = pk->fields.size();
    for (auto i : pk->fields)
    {
        field_append_value(&(_tbl->fields.at(i)), record.get(i), mutkey);
    }
    rocksdb::Slice k(mutkey.data(), mutkey.size());

    // build value
    //size_t pos = 0;
    for (auto it : _tbl->fields)
    {
        // TODO
        // check type
    }
    BufferArray ba;
    record.serialize(ba());
    rocksdb::Slice v(reinterpret_cast<const char *>(ba.data()), ba.size());

    // store
    auto ptr = RocksWrapper::get_instance();
    auto handle = ptr->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = ptr->put(wopt, handle, k, v);
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

int TableWriter::insert(const std::vector<TupleRecord> &records)
{
        auto ptr = RocksWrapper::get_instance();
    auto handle = ptr->get_data_handle();
    rocksdb::WriteBatch wbat;
    for (auto const &record : records)
    {
        // build key
        MutTableKey mutkey;
        mutkey.append_u32(_db->id);
        mutkey.append_u32(_tbl->id);
        auto pk = _tbl->get_primary_key();
        //size_t pk_num = pk->fields.size();
        for (auto i : pk->fields)
        {
            field_append_value(&(_tbl->fields.at(i)), record.get(i), mutkey);
        }
        rocksdb::Slice k(mutkey.data(), mutkey.size());

        // build value
        //size_t pos = 0;
        for (auto it : _tbl->fields)
        {
            // TODO
            // check type
        }
        BufferArray ba;
        record.serialize(ba());
        rocksdb::Slice v(reinterpret_cast<const char *>(ba.data()), ba.size());

        wbat.Put(handle, k, v);
    }

    // store
    rocksdb::WriteOptions wopt;
    auto status = ptr->write(wopt, &wbat);
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

int TableWriter::remove(const std::vector<Value> &row)
{
    // build key
    MutTableKey mutkey;
    mutkey.append_u32(_db->id);
    mutkey.append_u32(_tbl->id);
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();
    for (size_t i = 0; i < pk_num; ++i)
    {
        field_append_value(&(_tbl->fields.at(pk->fields.at(i))), &(row.at(i)), mutkey);
    }
    rocksdb::Slice k(mutkey.data(), mutkey.size());

    // store
    auto ptr = RocksWrapper::get_instance();
    auto handle = ptr->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = ptr->remove(wopt, handle, k);
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

int TableWriter::remove_range(const std::vector<Value> &start_key, const std::vector<Value> &end_key)
{
    auto pk = _tbl->get_primary_key();
    size_t pk_num = pk->fields.size();

    // build start key
    MutTableKey stkey;
    stkey.append_u32(_db->id);
    stkey.append_u32(_tbl->id);
    for (size_t i = 0; i < pk_num; ++i)
    {
        field_append_value(&(_tbl->fields.at(pk->fields.at(i))), &(start_key.at(i)), stkey);
    }
    rocksdb::Slice stk(stkey.data(), stkey.size());

    // build end key
    MutTableKey edkey;
    edkey.append_u32(_db->id);
    edkey.append_u32(_tbl->id);
    for (size_t i = 0; i < pk_num; ++i)
    {
        field_append_value(&(_tbl->fields.at(pk->fields.at(i))), &(end_key.at(i)), edkey);
    }
    rocksdb::Slice edk(edkey.data(), edkey.size());

    // store
    auto ptr = RocksWrapper::get_instance();
    auto handle = ptr->get_data_handle();
    rocksdb::WriteOptions wopt;
    auto status = ptr->remove_range(wopt, handle, stk, edk, false);
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

    auto ptr = RocksWrapper::get_instance();
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = ptr->seek_for_next(ptr->get_data_handle(), k, true);
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

    auto ptr = RocksWrapper::get_instance();
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = ptr->seek_for_prev(ptr->get_data_handle(), k, true);
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

    auto ptr = RocksWrapper::get_instance();
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = ptr->seek_for_next(ptr->get_data_handle(), k, prefix);
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
        TupleRecord trecord;
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


int TableReader::get(const std::vector<Value>& pk, TupleRecord& record)
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
    auto ptr = RocksWrapper::get_instance();
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    rocksdb::ReadOptions ropt;
    auto status = ptr->get(ropt, ptr->get_data_handle(), k, &value);
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
int TableReader::get(const std::vector<Value>& pk, std::vector<Value>& row)
{
    TupleRecord record;
    int rc = get(pk, record);
    if(rc != 0)
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

int TableReader::get(const Value& key, Value& val)
{
    auto pk = _tbl->get_primary_key();
    int rc = seek_to(pk, {key});
    if(rc != 0)
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
        TupleRecord trecord;
        trecord.deserialize(buf);

        for (auto it : _tbl->fields)
        {
            if(it.second.flags & static_cast<uint16_t>(FieldDescriptor::FieldFlag::PrimaryKey))
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