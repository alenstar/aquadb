/************************************************************************
Copyright 2017-2019 eBay Inc.
Author/Developer(s): Jung-Sang Ahn

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#include "log_store.h"

#include "libnuraft/nuraft.hxx"
#include "db/db.h"
#include "db/raft_log_store.h"
#include "service_impl.h"

#include <cassert>

namespace nuraft {


rocksdb_log_store::rocksdb_log_store(GlobalContext* ctx)
    : start_idx_(1), ctx_(ctx)
{
        // Dummy entry for index 0.
        ptr<buffer> buf = buffer::alloc(sz_ulong);
        dummy_entry_  = cs_new<log_entry>(0, buf);

    uint64_t index = 0;
    aquadb::Value value;
    int rc = ctx_->raft_log_store_->get_last(index, value);
    if(rc != 0)
    {
        LOGE("raft_log_store get last index fail, code=%d", rc);
        local_idx_ = 0;
    }
    else 
    {
        // TODO
        // FIXME (io error or not found key)
        // if io error then exit(1)
        // Dummy entry for index 0.
        LOGD("raft_log_store last index %lu", index);
        local_idx_ = index;
    }

    {
    uint64_t index = 1;
    aquadb::Value value;
    int rc = ctx_->raft_log_store_->get_first(index, value);
    if(rc != 0)
    {
        LOGE("raft_log_store get first index fail, code=%d", rc);
    }
    else 
    {
        LOGD("raft_log_store first index %lu", index);
        start_idx_ = index == 0 ? 1: index;
    }
    }
}

rocksdb_log_store::~rocksdb_log_store() {}

ptr<log_entry> rocksdb_log_store::make_clone(const ptr<log_entry>& entry) {
    ptr<log_entry> clone = cs_new<log_entry>
                           ( entry->get_term(),
                             buffer::clone( entry->get_buf() ),
                             entry->get_val_type() );
    return clone;
}

ulong rocksdb_log_store::next_slot() const {
    std::lock_guard<std::mutex> l(logs_lock_);
    // Exclude the dummy entry.
    //return start_idx_ + logs_.size() - 1;
    local_idx_ = get_local_last_idx();
    LOGD("local_idx: %lu, next: %lu", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(local_idx_ + 1));
    return 1 + local_idx_;
}

ulong rocksdb_log_store::start_index() const {
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_));
    return start_idx_;
}

ptr<log_entry> rocksdb_log_store::last_entry() const {
    // ulong next_idx = next_slot();
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_));

    uint64_t index = 0;
    aquadb::Value value;
   int rc =  ctx_->raft_log_store_->get_last(index, value);
   if(rc != 0)
   {
        ptr<buffer> buf = buffer::alloc(sz_ulong);
        return cs_new<log_entry>(0, buf);
   }
   auto buf = buffer::alloc(sizeof(uint64_t) + value.size());
        buf->put_raw(reinterpret_cast<const byte*>(value.data()), value.size());
        buf->pos(0);
   auto entry = log_entry::deserialize(*buf); 
   LOGD("last entry index %lu", index);
    return entry;
}

ulong rocksdb_log_store::append(ptr<log_entry>& entry) {
    LOGD("local_idx: %lu, start_idx: %lu, term: %lu, val_type: %d", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_), entry->get_term(), static_cast<int>(entry->get_val_type()));
    ptr<log_entry> clone = make_clone(entry);
    auto buf = clone->serialize();

    std::lock_guard<std::mutex> l(logs_lock_);
    size_t idx = start_idx_ + get_local_last_idx();

    LOGD("local_idx: %lu, start_idx: %lu buffer_size:%lu, data_size: %lu", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_), buf->size(), entry->get_buf().size());
    int rc = ctx_->raft_log_store_->put(idx, buf->data_begin(), buf->size());
    if(rc != 0)
    {
        LOGE("append entry to db fail, code=%d, idx=%lu", rc, idx);
    }
    return idx;
}

void rocksdb_log_store::write_at(ulong index, ptr<log_entry>& entry) {
    LOGD("index:%lu, term:%lu, val_type;%d", index, entry->get_term(), static_cast<int>(entry->get_val_type()));
    ptr<log_entry> clone = make_clone(entry);
    auto buf = clone->serialize();

    // Discard all logs equal to or greater than `index.
    std::lock_guard<std::mutex> l(logs_lock_);
    {
        uint64_t last_index = 0xffffffff;
        aquadb::Value value;
        int rc = ctx_->raft_log_store_->get_last(last_index, value);
        if(rc != 0)
        {
            LOGE("get last index fail, code=%d", rc);
        }
        if(last_index > index)
        {
            rc = ctx_->raft_log_store_->remove_range(index, last_index);
        }
    LOGD("index:%lu, last:%lu, term:%lu, val_type:%d", index, last_index, entry->get_term(), static_cast<int>(entry->get_val_type()));
        ctx_->raft_log_store_->put(index, reinterpret_cast<const char*>(buf->data_begin()), buf->size());

        local_idx_ = index;
    }
}

ptr< std::vector< ptr<log_entry> > >
    rocksdb_log_store::log_entries(ulong start, ulong end)
{
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_));
    LOGD("index:%lu ~ %lu", start, end);
    ptr< std::vector< ptr<log_entry> > > ret =
        cs_new< std::vector< ptr<log_entry> > >();
    ret->resize(end - start);

    ulong cc=0;
    auto fn = [&ret,&cc](uint64_t idx, const aquadb::Value& val)->bool{
        auto buf = buffer::alloc(val.size());
        buf->put_raw(reinterpret_cast<const byte*>(val.data()), val.size());
        buf->pos(0);
        auto entry = log_entry::deserialize(*buf);
    LOGD("index:%lu, term:%lu, val_type:%d", idx, entry->get_term(), static_cast<int>(entry->get_val_type()));
        (*ret)[cc++] = entry;
        return true;
    };
    int rc = ctx_->raft_log_store_->scan_range(start, end, fn);
    if(rc != 0)
    {
        LOGE("scan range fail, start: %lu, end: %lu", start, end);
    }
    return ret;
}

ptr<std::vector<ptr<log_entry>>>
    rocksdb_log_store::log_entries_ext(ulong start,
                                     ulong end,
                                     int64 batch_size_hint_in_bytes)
{
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_));
    ptr< std::vector< ptr<log_entry> > > ret =
        cs_new< std::vector< ptr<log_entry> > >();

    if (batch_size_hint_in_bytes < 0) {
        return ret;
    }

    size_t accum_size = 0;

    auto fn = [&ret,batch_size_hint_in_bytes, &accum_size](uint64_t idx, const aquadb::Value& val)->bool{
        auto buf = buffer::alloc(val.size());
        buf->put_raw(reinterpret_cast<const byte*>(val.data()), val.size());
        buf->pos(0);
        auto entry = log_entry::deserialize(*buf);
        ret->push_back(entry);
    LOGD("index:%lu, term:%lu, val_type:%d", idx, entry->get_term(), static_cast<int>(entry->get_val_type()));
        accum_size += entry->get_buf().size();
        if (batch_size_hint_in_bytes &&
            accum_size >= (ulong)batch_size_hint_in_bytes) { return false;}
        return true;
    };
    int rc = ctx_->raft_log_store_->scan_range(start, end, fn);
    if(rc != 0)
    {
        LOGE("scan range fail, start: %lu, end: %lu", start, end);
    }
    else {
        LOGD("scan range finished, start: %lu, end: %lu, %lu", start, end, ret->size());
    }

    return ret;
}

ptr<log_entry> rocksdb_log_store::entry_at(ulong index) {
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_));
    ptr<log_entry> src = nullptr;
       std::lock_guard<std::mutex> l(logs_lock_);

aquadb::Value value;
        int rc = ctx_->raft_log_store_->get(index, value);
    if(rc != 0)
    {
        LOGE("get raft log fail, index: %lu, code=%d", index, rc);
                ptr<buffer> buf = buffer::alloc(sz_ulong);
        return cs_new<log_entry>(0, buf);
    }
    else 
    {
        //auto buf = buffer::alloc(sizeof(uint64_t) + sizeof(char) + value.size());
        auto buf = buffer::alloc(value.size());
        buf->put_raw(reinterpret_cast<const byte*>(value.data()), value.size());
        buf->pos(0);
    //LOGD("index: %lu, local_idx: %lu, start_idx: %lu buffer_size: %lu value_size: %lu", index, static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_), buf->size(), value.size());
        auto entry = log_entry::deserialize(*buf);
    LOGD("local_idx: %lu, start_idx: %lu, term: %lu, val_type: %d", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_), entry->get_term(), static_cast<int>(entry->get_val_type()));
        return entry;
    }
}

ulong rocksdb_log_store::term_at(ulong index) {
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_));
    ulong term = 0;
     ptr<log_entry> entry = nullptr;

        aquadb::Value val;
        int rc = ctx_->raft_log_store_->get(index, val);
        if(rc != 0) {
            LOGE("get raft log fail, code=%d, index=%lu", rc, index);
            // TODO
            // seek to index lower_bound
            return 0;
        }
        else 
        {
        ptr<buffer> buf = buffer::alloc(val.size());
        buf->put_raw(reinterpret_cast<const byte*>(val.data()), val.size());
        buf->pos(0);
            entry = log_entry::deserialize(*buf);
    LOGD("local_idx: %lu, start_idx: %lu, term: %lu, val_type: %d", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_), entry->get_term(), static_cast<int>(entry->get_val_type()));
        }
        
    term = entry->get_term();
    return term;
}

ptr<buffer> rocksdb_log_store::pack(ulong index, int32 cnt) {
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_));
    std::vector<aquadb::Value> values;
    size_t size_total = 0;


auto fn = [&values,&size_total](uint64_t idx, const aquadb::Value& val) -> bool{
    values.push_back(val);
    size_total += val.size();
    return true;
};
        int rc = ctx_->raft_log_store_->scan_range(index, index + cnt,fn);
        if(rc != 0) {
            LOGE("get raft log fail, code=%d, index=%lu,%d", rc, index,cnt);
        return nullptr;
        }


    ptr<buffer> buf_out = buffer::alloc
                          ( sizeof(int32) +
                            cnt * sizeof(int32) +
                            size_total );
    buf_out->pos(0);
    buf_out->put((int32)cnt);


    for (auto& v: values) {
        buf_out->put((int32)v.size());
        buf_out->put(v.data(), v.size());
    }
    return buf_out;
}

void rocksdb_log_store::apply_pack(ulong index, buffer& pack) {
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(get_local_last_idx()), static_cast<uint64_t>(start_idx_));
    pack.pos(0);
    int32 num_logs = pack.get_int();
aquadb::RaftLogBundle bundle(ctx_->raft_log_store_.get());

        ulong cur_idx = 0;
    for (int32 ii=0; ii<num_logs; ++ii) {
        cur_idx = index + ii;
        int32 buf_size = pack.get_int();

        ptr<buffer> buf_local = buffer::alloc(buf_size);
        pack.get(buf_local);
        bundle.append(cur_idx, buf_local->data_begin(), buf_local->size());
    }
    int rc = ctx_->raft_log_store_->put(bundle);
            if(rc != 0)
            {
                LOGE("append raft log fail, code=%d, index=%lu,nums=%d", rc, index, num_logs);
                return;
            }

    {
    uint64_t index = 1;
    aquadb::Value value;
    int rc = ctx_->raft_log_store_->get_first(index, value);
    if(rc != 0)
    {
        LOGE("raft_log_store get first index fail, code=%d", rc);
    }
    else 
    {
        LOGD("raft_log_store first index %lu", index);
        std::lock_guard<std::mutex> l(logs_lock_);
        start_idx_ = index == 0 ? 1: index;
    }
    }

    {   
    uint64_t index = 1;
    aquadb::Value value;
    int rc = ctx_->raft_log_store_->get_last(index, value);
    if(rc != 0)
    {
        LOGE("raft_log_store get first index fail, code=%d", rc);
    }
    else 
    {
        LOGD("raft_log_store first index %lu", index);
        std::lock_guard<std::mutex> l(logs_lock_);
        local_idx_ = index == 0 ? 1: index;
    }
    }
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_));
}

bool rocksdb_log_store::compact(ulong last_log_index) {
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_));
            int rc = ctx_->raft_log_store_->remove_range(start_idx_, last_log_index);
            if(rc != 0)
            {
                LOGE("remove raft log fail, code=%d, index=%lu~%lu", rc, (size_t)(start_idx_), last_log_index);
                return false; 
            }

    // WARNING:
    //   Even though nothing has been erased,
    //   we should set `start_idx_` to new index.
        std::lock_guard<std::mutex> l(logs_lock_);
    if (start_idx_ <= last_log_index) {
        start_idx_ = last_log_index + 1;
    }
    LOGD("local_idx: %lu, start_idx: %lu", static_cast<uint64_t>(local_idx_), static_cast<uint64_t>(start_idx_));
    return true;
}

void rocksdb_log_store::close() {}

    ulong rocksdb_log_store::get_local_last_idx() const
    {
    uint64_t index = 0;
    aquadb::Value value;
    int rc = ctx_->raft_log_store_->get_last(index, value);
    if(rc != 0)
    {
        LOGE("raft_log_store get last index fail, code=%d", rc);
        //local_idx_ = 0;
    } 
    return index;
    }

}

