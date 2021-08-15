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

#include "state_machine.h"
#include "state_mgr.h"
#include "logger_wrapper.h"

#include "libnuraft/nuraft.hxx"
#include "db/db.h"
#include "db/raft_log_store.h"

#include "service_impl.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <gflags/gflags.h>

DEFINE_int32(server_id, 1, "server id");
DEFINE_int32(datacenter_id, 1, "datacenter id");
DEFINE_string(http,"0.0.0.0:9999", "http service listen address");
DEFINE_string(endpoint,"127.0.0.1:9900", "raft endpoint");
DEFINE_string(dbpath,"raft_data", "raft database path");

int init_raft(GlobalContext* ctx) {
    // Logger.
    std::string log_file_name = "./srv" +
                                std::to_string( ctx->server_id_ ) +
                                ".log";

    // uuid 
    ctx->order_id_gen_.init(ctx->server_id_, FLAGS_datacenter_id);
    ctx->trader_id_gen_.init(ctx->server_id_, FLAGS_datacenter_id);

    // aquadb
    ctx->db_mgr_ = aquadb::DBManager::get_instance();
    int rc = ctx->db_mgr_->init(FLAGS_dbpath);
    if(rc != 0)
    {
        LOGE("DBManager init %s fail, code=%d", FLAGS_dbpath.c_str(), rc);
        return -1;
    }
    rc = ctx->db_mgr_->open("db_raft", true);
    if(rc != 0)
    {
        LOGE("DBManager open db_raft fail, code=%d", rc);
        return rc;
    }
    //rc = ctx->db_mgr_->create_kv_table("db_raft","raft_log", aquadb::FieldDescriptor::FieldType::UInt64);
    //if(rc != 0)
    //{
    //    LOGE("DBManager create raft_log table fail, code=%d", rc);
    //    return rc;
    //}
    rc = ctx->db_mgr_->create_kv_table("db_raft","raft_config");
    if(rc != 0)
    {
        LOGE("DBManager create raft_config table fail, code=%d", rc);
        return rc;
    }
    // raft log store
    ctx->raft_log_store_ = std::make_shared<aquadb::RaftLogStore>(aquadb::RocksWrapper::get_instance());


    nuraft::ptr<logger_wrapper> log_wrap = nuraft::cs_new<logger_wrapper>( log_file_name, 4 );
    ctx->raft_logger_ = log_wrap;

    // State machine.
    ctx->smgr_ = nuraft::cs_new<nuraft::rocksdb_state_mgr>( ctx->server_id_,
                                           ctx->endpoint_, ctx);
    // State manager.
    ctx->sm_ = nuraft::cs_new<nuraft::rocksdb_state_machine>(); 

    // ASIO options.
    nuraft::asio_service::options asio_opt;
    asio_opt.thread_pool_size_ = 4;


    // Raft parameters.
    nuraft::raft_params params;
#if defined(WIN32) || defined(_WIN32)
    // heartbeat: 1 sec, election timeout: 2 - 4 sec.
    params.heart_beat_interval_ = 1000;
    params.election_timeout_lower_bound_ = 2000;
    params.election_timeout_upper_bound_ = 4000;
#else
    // heartbeat: 100 ms, election timeout: 200 - 400 ms.
    params.heart_beat_interval_ = 100;
    params.election_timeout_lower_bound_ = 200;
    params.election_timeout_upper_bound_ = 400;
#endif
    // Upto 5 logs will be preserved ahead the last snapshot.
    params.reserved_log_items_ = 5;
    // Snapshot will be created for every 5 log appends.
    params.snapshot_distance_ = 5;
    // Client timeout: 3000 ms.
    params.client_req_timeout_ = 3000;
    // According to this method, `append_log` function
    // should be handled differently.
    params.return_method_ = nuraft::raft_params::blocking;

    // Initialize Raft server.
    ctx->raft_instance_ = ctx->launcher_.init(ctx->sm_,
                                                ctx->smgr_,
                                                ctx->raft_logger_,
                                                ctx->port_,
                                                asio_opt,
                                                params);
    if (!ctx->raft_instance_) {
        std::cerr << "Failed to initialize launcher (see the message "
                     "in the log file)." << std::endl;
        log_wrap.reset();
        //exit(-1);
        return -1;
    }

    // Wait until Raft server is ready (upto 5 seconds).
    const size_t MAX_TRY = 20;
    LOGI("init Raft instance ");
    for (size_t ii=0; ii<MAX_TRY; ++ii) {
        if (ctx->raft_instance_->is_initialized()) {
            std::cout << " done" << std::endl;
            return 0;
        }
        std::cout << ".";
        fflush(stdout);
        //TestSuite::sleep_ms(250);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    LOGE("raft instance initialized FAILED");
    log_wrap.reset();
    //exit(-1);
    return -1;
}


int main(int argc, char** argv) {
    //if (argc < 3) usage(argc, argv);
   gflags::ParseCommandLineFlags(&argc, &argv, true);

    HttpServiceImpl impl;
    GlobalContext ctx;
    ctx.endpoint_ = FLAGS_endpoint;
    ctx.server_id_ = FLAGS_server_id;
    if(ctx.endpoint_.empty())
    {
        gflags::ShowUsageWithFlags(argv[0]);
        return -1;
    }
    {
        auto pos = FLAGS_endpoint.find(':');
        ctx.addr_ = FLAGS_endpoint.substr(0, pos);
        auto ss = FLAGS_endpoint.substr(pos + 1);
        ctx.port_ = std::atoi(ss.c_str());
    }

    std::cout << "    -- Replicated Calculator with Raft --" << std::endl;
    std::cout << "                         Version 0.1.0" << std::endl;
    std::cout << "    Server ID:    " << ctx.server_id_ << std::endl;
    std::cout << "    Endpoint:     " << ctx.addr_ << ":" << ctx.port_ << std::endl;
    std::cout << "    Server http:     " << FLAGS_http << std::endl;
    int rc = init_raft( &ctx );
    if(rc != 0)
    {
        std::cout << "init raft failed" << std::endl;
        return -1;
    }
    impl.Init(&ctx);
    //loop();
    // TODO
    // wait for shutdown
    impl.Listen(FLAGS_http);
    for(;;){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

