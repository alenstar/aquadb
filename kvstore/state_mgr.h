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

#pragma once

#include "util/filestream.h"
#include "util/logdef.h"
#include "log_store.h"
#include "in_memory_log_store.hxx"

#include "libnuraft/nuraft.hxx"

namespace nuraft {

class rocksdb_state_mgr: public state_mgr {
public:
    rocksdb_state_mgr(int srv_id,
                    const std::string& endpoint, GlobalContext* ctx)
        : my_id_(srv_id)
        , my_endpoint_(endpoint)
        , cur_log_store_( cs_new<rocksdb_log_store>(ctx) )
    {
        my_srv_config_ = cs_new<srv_config>( srv_id, endpoint );

        // Initial cluster config: contains only one server (myself).
        saved_config_ = cs_new<cluster_config>();
        saved_config_->get_servers().push_back(my_srv_config_);
    }

    ~rocksdb_state_mgr() {}
    std::string config_filename() const {
        return std::to_string(my_id_) + std::string("-cluster.config");
    }

    ptr<cluster_config> load_config() {
        // Just return in-memory data in this example.
        // May require reading from disk here, if it has been written to disk.
        util::FileStream fs(config_filename());
        if(fs.size()) 
        {
        auto buf = buffer::alloc(fs.size());
        fs.read(buf->data_begin(), fs.size());
        buf->pos(fs.size());
        saved_config_ = cluster_config::deserialize(*buf);
        }
        return saved_config_;
    }

    void save_config(const cluster_config& config) {
        // Just keep in memory in this example.
        // Need to write to disk here, if want to make it durable.
        ptr<buffer> buf = config.serialize();
        saved_config_ = cluster_config::deserialize(*buf);
    }

    void save_state(const srv_state& state) {
        // Just keep in memory in this example.
        // Need to write to disk here, if want to make it durable.
        ptr<buffer> buf = state.serialize();
        saved_state_ = srv_state::deserialize(*buf);
    }

    ptr<srv_state> read_state() {
        // Just return in-memory data in this example.
        // May require reading from disk here, if it has been written to disk.
        return saved_state_;
    }

    ptr<log_store> load_log_store() {
        return cur_log_store_;
    }

    int32 server_id() {
        return my_id_;
    }

    void system_exit(const int exit_code) {
    }

    ptr<srv_config> get_srv_config() const { return my_srv_config_; }

private:
    int my_id_;
    std::string my_endpoint_;
    ptr<rocksdb_log_store> cur_log_store_;
    ptr<srv_config> my_srv_config_;
    ptr<cluster_config> saved_config_;
    ptr<srv_state> saved_state_;
};

};

