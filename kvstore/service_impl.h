#pragma once
#include <iostream>

#include "state_machine.h"
#include "state_mgr.h"
#include "logger_wrapper.h"

#include "libnuraft/nuraft.hxx"
#include "httplib/httplib.h"
#include "nlohmann/json.hpp"

namespace aquadb{
    class BinlogRecord;
    class DBManager;
}

//struct  GlobalContext;
struct GlobalContext
{
    // Server ID.
    int server_id_;

    // Server address.
    std::string addr_;

    // Server port.
    int port_;

    // Endpoint: `<addr>:<port>`.
    std::string endpoint_;

    // Logger.
    nuraft::ptr<nuraft::logger> raft_logger_;

    // State machine.
    nuraft::ptr<nuraft::state_machine> sm_;

    // State manager.
    nuraft::ptr<nuraft::state_mgr> smgr_;

    // Raft launcher.
    nuraft::raft_launcher launcher_;

    // Raft server instance.
    nuraft::ptr<nuraft::raft_server> raft_instance_;

    aquadb::DBManager* db_mgr_{nullptr};

    nuraft::raft_params::return_method_type call_type_ =  nuraft::raft_params::blocking;
};


struct ServerRaftStatus {
    int server_id;
    int leader_id;
    uint64_t log_start_index;
    uint64_t log_end_index;
    uint64_t last_committed_index;

    nlohmann::json to_json() const {
        nlohmann::json js;
        js["server_id"] = server_id;
        js["leader_id"] = leader_id;
        js["log_start_index"] = log_start_index;
        js["log_end_index"] = log_end_index;
        js["last_committed_index"] = last_committed_index;
        return js;
    }
    std::string to_json_string() const {
        auto js = to_json();
        return js.dump();
    }
};


struct ServerRaftConfig
{
    int id;
    int dc_id;
    std::string endpoint;
    std::string aux;
    bool learner;
    bool leader;
    int priority;

    nlohmann::json to_json() const {
        nlohmann::json js;
        js["id"] = id;
        js["dc_id"] = dc_id;
        js["endpoint"] = endpoint;
        js["aux"] = aux;
        js["learner"] = learner;
        js["leader"] = leader;
        js["priority"] = priority;
        return js;
    }
    std::string to_json_string() const {
        auto js = to_json();
        return js.dump();
    }
};

class HttpServiceImpl
{
    public:
    // TODO
    int Init(GlobalContext* ctx);
    int Listen(const std::string& endpoint);

    // key format: <DB>.<TABLE>:<key>
    int Get(const std::string& k);
    int Put(const std::string& k, std::string& v);
    int Delete(const std::string& k);

    int Lock();
    int Unlock();
    int CreateLock();
    int DeleteLock();

    int Status(ServerRaftStatus& out);
    int AddServer(int server_id, const std::string& endpoint);
    int ServerList(std::vector<ServerRaftConfig>& out);

    int CreateTrader();
    int DeleteTrader();
    int EnterOrder();
    int CancelOrder();
    int OrderList();
    int PositionList();
    int TraderStatus();
    int UpdateQuotes();

    protected:
    void append_log(aquadb::BinlogRecord* record);
    private:
    GlobalContext* ctx_{nullptr};
    httplib::Server svr_;
};