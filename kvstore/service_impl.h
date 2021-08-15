#pragma once
#include <iostream>

#include "util/mutex.h"

#include "state_machine.h"
#include "state_mgr.h"
#include "logger_wrapper.h"

#include "libnuraft/nuraft.hxx"
#include "httplib/httplib.h"
#include "nlohmann/json.hpp"

#include "cerebro/cerebro_struct.h"
#include "cerebro/cerebro_trader.h"
#include "cerebro/snowflake.h"
#include "cerebro/mdlink.h"

#define KV_RAFT_LOG_TBL "raft_log"
#define KV_RAFT_CONFIG_TBL "raft_config"

using snowflake_t = snowflake<1627886931000L>;

namespace aquadb{
    class BinlogRecord;
    class DBManager;
    class TableOperator;
    class TableReader;
    class RaftLogStore;
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

    nuraft::raft_params::return_method_type call_type_ =  nuraft::raft_params::blocking;

    // uuid for trader
    snowflake_t trader_id_gen_;

    // uuid for order
    snowflake_t order_id_gen_;

    // rocksdb  
    aquadb::DBManager* db_mgr_{nullptr};
    std::shared_ptr<aquadb::RaftLogStore> raft_log_store_;
    std::shared_ptr<aquadb::TableOperator> get_raft_config_tbl();
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

    static nlohmann::json to_json(const std::vector<ServerRaftConfig>& cfgs) {
        nlohmann::json::array_t arr;
        for(auto const& cfg: cfgs)
        {
        nlohmann::json js = cfg.to_json();
        arr.emplace_back(std::move(js));
        }
        return arr;
        }
};

struct TradeOrderRequest
{
    int64_t trader_id = 0;
    std::string symbol; 
    int side = 0; // 1 buy, 2 sell
    double quantity; 
    double price = 0.0;

    int from_json(nlohmann::json& js) {
        trader_id= js.at("trader_id").get<int64_t>();
        symbol = js.at("symbol").get<std::string>();
        side = js.at("side").get<int>();
        quantity = js.at("quantity").get<double>();
        price = js.at("price").get<double>();
        return 0;
    }
    nlohmann::json to_json() const {
        nlohmann::json js;
        js["trader_id"] = trader_id;
        js["symbol"] = symbol;
        js["side"] = side;
        js["quantity"] = quantity;
        js["price"] = price;
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
    int RemoveServer(int server_id);
    int ServerList(std::vector<ServerRaftConfig>& out);

    int GetQuotes(const std::vector<std::string>& codes, std::vector<CerebroTickRecordPtr>& records);
    int SubscribeQuotes(const std::vector<std::string>& codes);
    CerebroTraderPtr CreateTrader(const std::string& name, double cash);
    int DeleteTrader(int64_t id);
    CerebroTraderPtr GetTrader(int64_t id);
    CerebroTraderPtr GetTrader(const std::string& name);
    int64_t EnterOrder(const TradeOrderRequest& req);
    int64_t CancelOrder(int64_t trader_id, int64_t order_id);
    int OrderList(int64_t trader_id, std::vector<CerebroOrder>& orders);
    int PositionList(int64_t trader_id, std::vector<CerebroPosition>& positions);

    protected:
    void append_log(aquadb::BinlogRecord* record);
    private:
    GlobalContext* ctx_{nullptr};
    httplib::Server svr_;

    std::map<int64_t, CerebroTraderPtr> traders_;
    std::map<std::string, int64_t> traders_nm_;
    util::RWMutex traders_mtx_;    

    // marketdata link
    mdlink_api::MarketDataProvider mdlink_;
    std::map<std::string, CerebroTickRecordPtr> quotes_;
    util::RWMutex quotes_mtx_;    

};