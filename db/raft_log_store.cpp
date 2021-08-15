#include "raft_log_store.h"
#include "table_key.h"
#include "util/logdef.h"
namespace aquadb
{

        RaftLogBundle::RaftLogBundle(RaftLogStore* store)
        {
            handle_ = store->wrapper_->get_raft_log_handle();
        }
int RaftLogBundle::append(uint64_t index, const Value& value)
{
    MutTableKey mutkey;
    mutkey.append_u64(index);
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    rocksdb::Slice v(reinterpret_cast<const char *>(value.data()), value.size());
    auto status = wbatch_.Put(handle_, k, v);
    if(status.ok())
    {
        return 0;
    }
    else 
    {
        std::string errmsg = status.ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return static_cast<int>(status.code());
    }
}

int RaftLogBundle::append(uint64_t index, const void* data, size_t size)
{
    MutTableKey mutkey;
    mutkey.append_u64(index);
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    rocksdb::Slice v(reinterpret_cast<const char*>(data), size);
    auto status = wbatch_.Put(handle_, k, v);
    if(status.ok())
    {
        return 0;
    }
    else 
    {
        std::string errmsg = status.ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return static_cast<int>(status.code());
    }
}


RaftLogStore::RaftLogStore(RocksWrapper *wrapper) : wrapper_(wrapper) {}
RaftLogStore::~RaftLogStore() {}

int RaftLogStore::put(uint64_t index, const Value &value)
{

    // build key
    MutTableKey mutkey;
    mutkey.append_u64(index);
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    rocksdb::Slice v(reinterpret_cast<const char *>(value.data()), value.size());

    // store
    rocksdb::WriteOptions wopt;
    auto status = wrapper_->put(wopt, wrapper_->get_raft_log_handle(), k, v);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        std::string errmsg = status.ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return static_cast<int>(status.code());
    }
}
int RaftLogStore::put(uint64_t index, const void* data, size_t size)
{
    // build key
    MutTableKey mutkey;
    mutkey.append_u64(index);
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    rocksdb::Slice v(reinterpret_cast<const char*>(data), size);

    // store
    rocksdb::WriteOptions wopt;
    auto status = wrapper_->put(wopt, wrapper_->get_raft_log_handle(), k, v);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        std::string errmsg = status.ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return static_cast<int>(status.code());
    }
}

int RaftLogStore::put(RaftLogBundle& bundle)
{
    // store
    rocksdb::WriteOptions wopt;
    auto status = wrapper_->write(wopt, bundle.data());
    if (status.ok())
    {
        return 0;
    }
    else
    {
        std::string errmsg = status.ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return static_cast<int>(status.code());
    }
}

int RaftLogStore::get(uint64_t index, Value &value)
{
    MutTableKey mutkey;
    mutkey.append_u64(index);
    rocksdb::Slice k(mutkey.data(), mutkey.size());
    auto cursor = wrapper_->seek_for_next( wrapper_->get_raft_log_handle(), k, false);

    if (!cursor->Valid())
    {
        // error
        std::string errmsg = cursor->status().ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return static_cast<int>(cursor->status().code());
    }

    auto v = cursor->value();
    value.set_value(v.data(), v.size());
    return 0;
}


int RaftLogStore::scan_range(uint64_t start_index, uint64_t end_index,
                             std::function<bool(uint64_t idx, const Value &val)> fn)
{
    // build start key
    MutTableKey stkey;
    stkey.append_u64(start_index);
    rocksdb::Slice stk(stkey.data(), stkey.size());

    // build end key
    MutTableKey edkey;
    edkey.append_u64(end_index);
    rocksdb::Slice edk(edkey.data(), edkey.size());

    // store
    rocksdb::Slice k(stkey.data(), stkey.size());
    rocksdb::Slice endk(edkey.data(), edkey.size());
    auto cursor = wrapper_->seek_for_next( wrapper_->get_raft_log_handle(), k, false);
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

        auto v = cursor->value();
        
        TableKey key(k.data(),k.size());
        uint64_t index = key.extract_u64(0);
        Value val(v.data(), v.size());

        if (!fn(index, val))
        {
            break;
        }
        cursor->Next();
    }

    return 0;
}


int RaftLogStore::get_first(uint64_t& index,Value &value)
{
    rocksdb::ReadOptions roption(false, true);
    roption.fill_cache = false;
    std::unique_ptr<rocksdb::Iterator> cursor(wrapper_->new_iterator(roption, wrapper_->get_raft_log_handle()));
    cursor->SeekToFirst();
    if (!cursor->status().ok())
    {
        // error
        std::string errmsg = cursor->status().ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return ERR_INVALID_PARAMS;
    }
    if(!cursor->Valid())
    {
        return ERR_NOT_FOUND;
    }
    auto k = cursor->key();
        TableKey key(k.data(),k.size());
        index = key.extract_u64(0);

    auto v = cursor->value();
    value.set_value(v.data(), v.size());
    return 0;
}


int RaftLogStore::get_last(uint64_t& index,Value &value)
{
    rocksdb::ReadOptions roption(false, true);
    roption.fill_cache = false;
    std::unique_ptr<rocksdb::Iterator> cursor(wrapper_->new_iterator(roption, wrapper_->get_raft_log_handle()));
    cursor->SeekToLast();
    if (!cursor->status().ok())
    {
        // error
        std::string errmsg = cursor->status().ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return ERR_INVALID_PARAMS;
    }
    if(!cursor->Valid())
    {
        return ERR_NOT_FOUND;
    }
    auto k = cursor->key();
        TableKey key(k.data(),k.size());
        index = key.extract_u64(0);

    auto v = cursor->value();
    value.set_value(v.data(), v.size());
    return 0;
}

int RaftLogStore::remove(uint64_t index) {
        // build key
    MutTableKey mutkey;
    mutkey.append_u64(index);
    rocksdb::Slice k(mutkey.data(), mutkey.size());

    // store
    rocksdb::WriteOptions wopt;
    auto status = wrapper_->remove(wopt, wrapper_->get_raft_log_handle(), k);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        std::string errmsg = status.ToString();
        LOGE("rocksdb::iterator invalid:%s", errmsg.c_str());
        return static_cast<int>(status.code());
    }
    return 0;
}
int RaftLogStore::remove_range(uint64_t start_index, uint64_t end_index)
{
            // build key
    MutTableKey start_key;
    start_key.append_u64(start_index);
    rocksdb::Slice sk(start_key.data(), start_key.size());

    MutTableKey end_key;
    end_key.append_u64(end_index);
    rocksdb::Slice ek(end_key.data(), end_key.size());

    // store
    rocksdb::WriteOptions wopt;
    auto status = wrapper_->remove_range(wopt, wrapper_->get_raft_log_handle(), sk,ek ,false);
    if (status.ok())
    {
        return 0;
    }
    else
    {
        std::string errmsg = status.ToString();
        LOGE("rocksdb::iterator invalid:%s (%lu,%lu)", errmsg.c_str(), start_index, end_index);
        return static_cast<int>(status.code());
    }
    return 0;
}

}