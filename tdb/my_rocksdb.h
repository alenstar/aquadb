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

#include "rocks_wrapper.h"
#include <butil/time.h>

namespace tdb {

class TimeCost {
public:
    TimeCost() {
        _start = butil::gettimeofday_us();
    }

    ~TimeCost() {}

    void reset() {
        _start = butil::gettimeofday_us();
    }

    int64_t get_time() const {
        return butil::gettimeofday_us() - _start;
    }

private:
    int64_t _start;
};

struct RocksdbVars {
    static RocksdbVars* get_instance() {
        static RocksdbVars _instance;
        return &_instance;
    }

    bvar::LatencyRecorder    rocksdb_put_time_cost;
    bvar::LatencyRecorder    rocksdb_get_time_cost;
    bvar::LatencyRecorder    rocksdb_scan_time_cost;
    bvar::LatencyRecorder    rocksdb_seek_time_cost;
    bvar::LatencyRecorder    qos_fetch_tokens_wait_time_cost;
    bvar::Adder<int64_t>     qos_fetch_tokens_wait_count;
    bvar::Adder<int64_t>     qos_fetch_tokens_count;
    bvar::PerSecond<bvar::Adder<int64_t> > qos_fetch_tokens_qps;
    bvar::Adder<int64_t>     qos_token_waste_count;
    bvar::PerSecond<bvar::Adder<int64_t> > qos_token_waste_qps;
    // 统计未提交的binlog最大时间
    bvar::Maxer<int64_t>     binlog_not_commit_max_cost;
    bvar::Window<bvar::Maxer<int64_t> > binlog_not_commit_max_cost_minute;

private:
    RocksdbVars(): rocksdb_put_time_cost("rocksdb_put_time_cost"),
                   rocksdb_get_time_cost("rocksdb_get_time_cost"),
                   rocksdb_scan_time_cost("rocksdb_scan_time_cost"),
                   rocksdb_seek_time_cost("rocksdb_seek_time_cost"),
                   qos_fetch_tokens_wait_time_cost("qos_fetch_tokens_wait_time_cost"),
                   qos_fetch_tokens_wait_count("qos_fetch_tokens_wait_count"),
                   qos_fetch_tokens_qps("qos_fetch_tokens_qps", &qos_fetch_tokens_count),
                   qos_token_waste_qps("qos_token_waste_qps", &qos_token_waste_count),
                   binlog_not_commit_max_cost_minute("binlog_not_commit_max_cost_minute", &binlog_not_commit_max_cost, 60) {
                   }
};

namespace myrocksdb {

class Iterator {
public:
    explicit Iterator(rocksdb::Iterator* iter) : _iter(iter) { }

    virtual ~Iterator() { delete _iter; }

    bool Valid() { return _iter->Valid(); }

    void Seek(const rocksdb::Slice& target);

    void SeekForPrev(const rocksdb::Slice& target);

    void Next();

    void Prev();

    rocksdb::Slice key()   { return _iter->key(); }

    rocksdb::Slice value() { return _iter->value(); }

private:
    rocksdb::Iterator* _iter = nullptr;
};

class Transaction {
public:
    explicit Transaction(rocksdb::Transaction* txn) : _txn(txn) { }

    virtual ~Transaction() { delete _txn; }

    // rocksdb::Transaction* get_txn() { return _txn; }

    // rocksdb::Status Get(const rocksdb::ReadOptions& options, const rocksdb::Slice& key,
    //                     std::string* value) {
    //     return _txn->Get(options, key, value);
    // }

    // rocksdb::Status Get(const rocksdb::ReadOptions& options, const rocksdb::Slice& key,
    //                     rocksdb::PinnableSlice* pinnable_val) {
    //     return _txn->Get(options, key, pinnable_val);
    // }

    rocksdb::Status Get(const rocksdb::ReadOptions& options,
                     rocksdb::ColumnFamilyHandle* column_family, const rocksdb::Slice& key,
                     std::string* value);

    rocksdb::Status Get(const rocksdb::ReadOptions& options,
                     rocksdb::ColumnFamilyHandle* column_family, const rocksdb::Slice& key,
                     rocksdb::PinnableSlice* pinnable_val);

    rocksdb::Status GetForUpdate(const rocksdb::ReadOptions& options,
                              rocksdb::ColumnFamilyHandle* column_family,
                              const rocksdb::Slice& key, std::string* value);

    rocksdb::Status GetForUpdate(const rocksdb::ReadOptions& options,
                              rocksdb::ColumnFamilyHandle* column_family,
                              const rocksdb::Slice& key, rocksdb::PinnableSlice* pinnable_val);

    // rocksdb::Status Put(const rocksdb::Slice& key, const rocksdb::Slice& value) {
    //     return _txn->Put(key, value);
    // }

    // rocksdb::Status Put(const rocksdb::SliceParts& key, const rocksdb::SliceParts& value) {
    //     return _txn->Put(key, value);
    // }

    rocksdb::Status Put(rocksdb::ColumnFamilyHandle* column_family, const rocksdb::Slice& key,
                     const rocksdb::Slice& value);

    rocksdb::Status Put(rocksdb::ColumnFamilyHandle* column_family, const rocksdb::SliceParts& key,
                     const rocksdb::SliceParts& value);

    rocksdb::Status Delete(rocksdb::ColumnFamilyHandle* column_family, const rocksdb::Slice& key) {
        return _txn->Delete(column_family, key);
    }

    rocksdb::Status Delete(rocksdb::ColumnFamilyHandle* column_family,
                        const rocksdb::SliceParts& key) {
        return _txn->Delete(column_family, key);
    }

    rocksdb::Status SetName(const rocksdb::TransactionName& name) { return _txn->SetName(name); }

    rocksdb::TransactionName GetName() const { return _txn->GetName(); }

    rocksdb::TransactionID GetID() const { return _txn->GetID(); }

    rocksdb::Iterator* GetIterator(const rocksdb::ReadOptions& read_options) {
        return _txn->GetIterator(read_options);
    }

    rocksdb::Iterator* GetIterator(const rocksdb::ReadOptions& read_options,
                                rocksdb::ColumnFamilyHandle* column_family) {
        return _txn->GetIterator(read_options, column_family);
    }

    rocksdb::Status Prepare()  { return _txn->Prepare(); }

    rocksdb::Status Commit()   { return _txn->Commit(); }

    rocksdb::Status Rollback() { return _txn->Rollback(); }

    void SetSavePoint() { _txn->SetSavePoint(); }

    rocksdb::Status RollbackToSavePoint() { return _txn->RollbackToSavePoint(); }

    std::vector<rocksdb::TransactionID> GetWaitingTxns(uint32_t* column_family_id,
                                                        std::string* key) const {
          return _txn->GetWaitingTxns(column_family_id, key);
    }

    void DisableIndexing() { _txn->DisableIndexing(); }

private:
    rocksdb::Transaction* _txn = nullptr;
};

} // namespace myrocksdb
} // namespace tdb
