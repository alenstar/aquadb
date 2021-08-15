#pragma once
#include <memory>
#include <functional>

#include <rocksdb/slice.h>
#include "db/rocks_wrapper.h"
#include "db.h"
#include "key_encoder.h"

namespace aquadb
{

class RaftLogStore;
class RaftLogBundle{
        public:
        RaftLogBundle(RaftLogStore* store);
        ~RaftLogBundle() = default;
        int append(uint64_t index, const Value& value);
        int append(uint64_t index, const void* data, size_t size);
        rocksdb::WriteBatch* data()  {return &wbatch_;}
        private:
        rocksdb::WriteBatch wbatch_;
        rocksdb::ColumnFamilyHandle* handle_{nullptr};
};

class RaftLogStore
{
    public:
    RaftLogStore(RocksWrapper* wrapper);
    ~RaftLogStore();
    int put(uint64_t index, const Value& value);
    int put(uint64_t index, const void* data, size_t size);
    int put(RaftLogBundle& bundle);
    int get(uint64_t index, Value& value);
    int scan_range(uint64_t start_index, uint64_t end_index, std::function<bool(uint64_t idx, const Value& val)> fn);
    int get_first(uint64_t &index,Value& value);
    int get_last(uint64_t &index, Value& value);
    int remove(uint64_t index);
    int remove_range(uint64_t start_index, uint64_t end_index);
    private:
    friend RaftLogBundle;
    RocksWrapper* wrapper_{nullptr};
};
typedef std::shared_ptr<RaftLogStore> RaftLogStorePtr;

}