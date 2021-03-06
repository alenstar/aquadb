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

#include "my_rocksdb.h"
//#include "qos.h"

namespace aquadb {

namespace myrocksdb {

void Iterator::Seek(const rocksdb::Slice& target) {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->get_rate_limiting();
    //}

    TimeCost cost;
    // 执行
    _iter->Seek(target);

    // 统计
    //if (local != nullptr) {
    //    local->get_statistics_adder();
    //}

    //RocksdbVars::get_instance()->rocksdb_seek_time_cost << cost.get_time();

}

void Iterator::SeekForPrev(const rocksdb::Slice& target) {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->get_rate_limiting();
    //}

    TimeCost cost;
    // 执行
    _iter->SeekForPrev(target);

    // 统计
    //if (local != nullptr) {
    //    local->get_statistics_adder();
    //}

    //RocksdbVars::get_instance()->rocksdb_seek_time_cost << cost.get_time();
}

void Iterator::Next() {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->scan_rate_limiting();
    //}

    TimeCost cost;
    // 执行
    _iter->Next();

    // 统计
    //if (local != nullptr) {
    //    local->scan_statistics_adder();
    //}

    //RocksdbVars::get_instance()->rocksdb_scan_time_cost << cost.get_time();

    return;
}

void Iterator::Prev() {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->scan_rate_limiting();
    //}

    // 执行
    TimeCost cost;
    _iter->Prev();

    // 统计
    //if (local != nullptr) {
    //    local->scan_statistics_adder();
    //}

    //RocksdbVars::get_instance()->rocksdb_scan_time_cost << cost.get_time();

    return;
}

rocksdb::Status Transaction::Get(const rocksdb::ReadOptions& options,
                    rocksdb::ColumnFamilyHandle* column_family, const rocksdb::Slice& key,
                    std::string* value) {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->get_rate_limiting();
    //}

    // 执行
    TimeCost cost;
    auto s = _txn->Get(options, column_family, key, value);

    // 统计
    //if (local != nullptr) {
    //    local->get_statistics_adder();
    //}

    // RocksdbVars::get_instance()->rocksdb_get_time_cost << cost.get_time();

    return s;
}

rocksdb::Status Transaction::Get(const rocksdb::ReadOptions& options,
                    rocksdb::ColumnFamilyHandle* column_family, const rocksdb::Slice& key,
                    rocksdb::PinnableSlice* pinnable_val) {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->get_rate_limiting();
    //}

    // 执行
    TimeCost cost;
    auto s = _txn->Get(options, column_family, key, pinnable_val);

    // 统计
    //if (local != nullptr) {
    //    local->get_statistics_adder();
    //}

    // RocksdbVars::get_instance()->rocksdb_get_time_cost << cost.get_time();

    return s;
}

rocksdb::Status Transaction::GetForUpdate(const rocksdb::ReadOptions& options,
                            rocksdb::ColumnFamilyHandle* column_family,
                            const rocksdb::Slice& key, std::string* value) {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->get_rate_limiting();
    //}

    // 执行
    TimeCost cost;
    auto s = _txn->GetForUpdate(options, column_family, key, value);

    // 统计
    //if (local != nullptr) {
    //    local->get_statistics_adder();
    //}

    // RocksdbVars::get_instance()->rocksdb_get_time_cost << cost.get_time();

    return s;
}

rocksdb::Status Transaction::GetForUpdate(const rocksdb::ReadOptions& options,
                            rocksdb::ColumnFamilyHandle* column_family,
                            const rocksdb::Slice& key, rocksdb::PinnableSlice* pinnable_val) {
    //QosBthreadLocal* local = StoreQos::get_instance()->get_bthread_local();

    // 限流
    //if (local != nullptr) {
    //    local->get_rate_limiting();
    //}

    // 执行
    TimeCost cost;
    auto s = _txn->GetForUpdate(options, column_family, key, pinnable_val);

    // 统计
    //if (local != nullptr) {
    //    local->get_statistics_adder();
    //}

    // RocksdbVars::get_instance()->rocksdb_get_time_cost << cost.get_time();

    return s;
}

rocksdb::Status Transaction::Put(rocksdb::ColumnFamilyHandle* column_family, const rocksdb::Slice& key,
                    const rocksdb::Slice& value) {
    TimeCost cost;
    auto s = _txn->Put(column_family, key, value);

    // RocksdbVars::get_instance()->rocksdb_put_time_cost << cost.get_time();

    return s;
}

rocksdb::Status Transaction::Put(rocksdb::ColumnFamilyHandle* column_family, const rocksdb::SliceParts& key,
                    const rocksdb::SliceParts& value) {
    TimeCost cost;
    auto s = _txn->Put(column_family, key, value);

    // RocksdbVars::get_instance()->rocksdb_put_time_cost << cost.get_time();

    return s;
}

} // namespace myrocksdb
} // namespace aquadb
