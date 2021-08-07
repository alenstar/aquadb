#include "service_impl.h"
#include "db/db.h"
#include "nlohmann/json.hpp"

#include "state_machine.h"
#include "state_mgr.h"

#define GET_STATE_MACHINE(x) (static_cast<nuraft::rocksdb_state_machine *>(x->sm_.get()))

using raft_result = nuraft::cmd_result<nuraft::ptr<nuraft::buffer>>;

int HttpServiceImpl::Init(GlobalContext *ctx)
{
    ctx_ = ctx;
    mdlink_.init(4);
    mdlink_.set_quotes_callback([this](CerebroTickRecordPtr q)->bool{
        std::cerr << "quote:" << *q << std::endl;

        {
        util::RWMutex::WriteLock lck(quotes_mtx_);
        quotes_[q->symbol] = q;
        }
        // on_tick for broker
        {
            util::RWMutex::ReadLock lck(traders_mtx_);
            for(auto it: traders_)
            {
                it.second->get_broker()->on_tick(*q);
            }
        }
        return true;
    });

    svr_.set_logger([](const httplib::Request &req, const httplib::Response &res)->void{
        LOGI("path=%s", req.path.c_str());
    });

    //////////////////////////////////////////////////////////////////////////////////////
    svr_.Get("/raft/status", [this](const httplib::Request &req, httplib::Response &res) {
        nlohmann::json js;
        ServerRaftStatus out;
        int rc = Status(out);
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            js["data"] = out.to_json();
        }
        res.set_content(js.dump(), "application/json");
    });

    svr_.Get("/raft/serverlist", [this](const httplib::Request &req, httplib::Response &res) {
        nlohmann::json js;
        std::vector<ServerRaftConfig> out;
        int rc = ServerList(out);
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            if (out.empty())
            {
                js["data"] = nullptr;
            }
            else
            {
                js["data"] = nlohmann::json::array();
                for (auto &c : out)
                {
                    js["data"].push_back(c.to_json());
                }
            }
        }
        res.set_content(js.dump(), "application/json");
    });

    svr_.Post("/raft/addserver", [this](const httplib::Request &req, httplib::Response &res) {
        nlohmann::json js = nlohmann::json::parse(req.body);
        std::string out;
        int rc = AddServer(js.at("server_id").get<int>(), js.at("endpoint").get<std::string>());
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            //    js["data"] = out.to_json();
        }
        res.set_content(js.dump(), "application/json");
    });
    svr_.Post("/raft/removeserver", [this](const httplib::Request &req, httplib::Response &res) {
        nlohmann::json js = nlohmann::json::parse(req.body);
        std::string out;
        int rc = RemoveServer(js.at("server_id").get<int>());
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            //    js["data"] = out.to_json();
        }
        res.set_content(js.dump(), "application/json");
    });

    ///////////////////////////////////////////////////////////////////////////////////////////
    svr_.Get("/quote/quotes", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js;
        std::string code = req.get_param_value("codes"); 
        std::vector<CerebroTickRecordPtr> records;
        int rc = GetQuotes({code}, records);
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            nlohmann::json::array_t arr;
            for(auto& q: records)
            {
                arr.emplace_back(q->to_json());
            }
            js["data"] = std::move(arr);
        }
        res.set_content(js.dump(), "application/json");
    });
    svr_.Post("/quote/subscribe", [this](const httplib::Request &req, httplib::Response &res){
        std::vector<std::string> codes;
        nlohmann::json js = nlohmann::json::parse(req.body);
        auto c = js.at("codes");
        if(c.is_array())
        {
            for(size_t i = 0; i < c.size(); ++i)
            {
                codes.emplace_back(c[i].get<std::string>());
            }
        }
        else if(c.is_string()){
            codes.emplace_back(c.get<std::string>());
        }
        std::string out;
        int rc = SubscribeQuotes(codes);
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            //    js["data"] = out.to_json();
        }
        res.set_content(js.dump(), "application/json");
    });

    /////////////////////////////////////////////////////////////////

    svr_.Get("/trade/trader", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js;
        CerebroTraderPtr trader = nullptr;
        {
            if(req.has_param("trader_id"))
            {
        auto trader_id = util::strto<int64_t>(req.get_param_value("trader_id")); 
        trader = GetTrader(trader_id);
            }
            else if(req.has_param("trader_name")) {
        auto trader_name = req.get_param_value("trader_name"); 
        trader = GetTrader(trader_name);
            }
            else 
            {
            js["ret_code"] = -1;
            js["err_msg"] = "invalid param: not found trader_id or trader_name";
        res.set_content(js.dump(), "application/json");
        return;
            }
        }
        if (trader == nullptr)
        {
            js["ret_code"] = -1;
            js["err_msg"] = "not found the trader";
        }
        else
        {
            js["ret_code"] = 0;
            js["data"] = trader->to_json();
        }
        res.set_content(js.dump(), "application/json");
    });

    svr_.Post("/trade/createtrader", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js = nlohmann::json::parse(req.body);
        auto trader = CreateTrader(js.at("name").get<std::string>(), js.at("cash").get<double>());

        js.clear();
        js["ret_code"] = trader ? 0:-1;
        if (trader == nullptr)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            js["data"] = trader->to_json();
        }
        res.set_content(js.dump(), "application/json");
    });
    svr_.Post("/trade/deletetrader", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js = nlohmann::json::parse(req.body);
        int rc = DeleteTrader(js.at("id").get<int64_t>());

        js.clear();
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            // js["data"] = nullptr;
        }
        res.set_content(js.dump(), "application/json");
    });

    svr_.Post("/trade/enterorder", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js = nlohmann::json::parse(req.body);
        TradeOrderRequest oreq;
        oreq.from_json(js);
        int64_t rc = EnterOrder(oreq);

        js.clear();
        if (rc < 1)
        {
            js["ret_code"] = -1;
            js["err_msg"] = "internal error";
        }
        else
        {
            js["ret_code"] = 0;
            nlohmann::json subjs;
            subjs["order_id"] = rc;
            js["data"] = subjs;
        }
        res.set_content(js.dump(), "application/json");
    });
    svr_.Post("/trade/cancelorder", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js = nlohmann::json::parse(req.body);
        int64_t rc = CancelOrder(js.at("trader_id").get<int64_t>(), js.at("order_id").get<int64_t>());

        js.clear();
        if (rc < 1)
        {
            js["ret_code"] = -1;
            js["err_msg"] = "internal error";
        }
        else
        {
            js["ret_code"] = 0;
            nlohmann::json subjs;
            subjs["order_id"] = rc;
            js["data"] = subjs;
        }
        res.set_content(js.dump(), "application/json");
    });

    svr_.Get("/trade/orderlist", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js;
        auto trader_id = util::strto<int64_t>(req.get_param_value("trader_id")); 
        std::vector<CerebroOrder> records;
        int rc = OrderList(trader_id, records);
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            nlohmann::json::array_t arr = CerebroOrder::to_json(records);
            js["data"] = std::move(arr);
        }
        res.set_content(js.dump(), "application/json");
    });
    svr_.Get("/trade/positionlist", [this](const httplib::Request &req, httplib::Response &res){
        nlohmann::json js;
        auto trader_id = util::strto<int64_t>(req.get_param_value("trader_id")); 
        std::vector<CerebroPosition> records;
        int rc = PositionList(trader_id, records);
        js["ret_code"] = rc;
        if (rc != 0)
        {
            js["err_msg"] = "internal error";
        }
        else
        {
            nlohmann::json::array_t arr = CerebroPosition::to_json(records);
            js["data"] = std::move(arr);
        }
        res.set_content(js.dump(), "application/json");
    });
    return 0;
}
int HttpServiceImpl::Listen(const std::string &endpoint)
{
    auto pos = endpoint.find(':');
    std::string host = endpoint.substr(0, pos);
    std::string sport = endpoint.substr(pos + 1);
    int port = std::atoi(sport.c_str());
    bool ok = svr_.listen(host.c_str(), port);
    return (ok ? 0 : -1);
}

// key format: <DB>.<TABLE>:<key>
int HttpServiceImpl::Get(const std::string &k) { return -1; }
int HttpServiceImpl::Put(const std::string &k, std::string &v) { return -1; }
int HttpServiceImpl::Delete(const std::string &k) { return -1; }

int HttpServiceImpl::Lock() { return -1; }
int HttpServiceImpl::Unlock() { return -1; }
int HttpServiceImpl::CreateLock() { return -1; }
int HttpServiceImpl::DeleteLock() { return -1; }

int HttpServiceImpl::Status(ServerRaftStatus &out)
{
    nuraft::ptr<nuraft::log_store> ls = ctx_->smgr_->load_log_store();
    std::stringstream ss;
    out.server_id = ctx_->server_id_;
    out.leader_id = ctx_->raft_instance_->get_leader();
    out.log_start_index = ls->start_index();
    out.log_end_index = ls->next_slot() - 1;
    out.last_committed_index = ctx_->raft_instance_->get_committed_log_idx();
    // "state machine value: " << GET_STATE_MACHINE(ctx_)->get_current_value() << "\n";
    return 0;
}

int HttpServiceImpl::ServerList(std::vector<ServerRaftConfig> &out)
{
    std::vector<nuraft::ptr<nuraft::srv_config>> configs;
    ctx_->raft_instance_->get_srv_config_all(configs);

    int leader_id = ctx_->raft_instance_->get_leader();

    std::stringstream ss;
    for (auto &entry : configs)
    {
        ServerRaftConfig cfg;
        nuraft::ptr<nuraft::srv_config> &srv = entry;
        cfg.id = srv->get_id();
        cfg.dc_id = srv->get_dc_id();
        cfg.endpoint = srv->get_endpoint();
        cfg.aux = srv->get_aux();
        cfg.learner = srv->is_learner();
        if (srv->get_id() == leader_id)
        {
            cfg.leader = true;
        }
        else
        {
            cfg.leader = false;
        }
        cfg.priority = srv->get_priority();
        out.emplace_back(cfg);
    }
    return 0;
}


int HttpServiceImpl::AddServer(int server_id, const std::string &endpoint)
{

    if (!server_id || server_id == ctx_->server_id_)
    {
        std::cout << "wrong server id: " << server_id << std::endl;
        return -1;
    }

    nuraft::srv_config srv_conf_to_add(server_id, endpoint);
    nuraft::ptr<raft_result> ret = ctx_->raft_instance_->add_srv(srv_conf_to_add);
    if (!ret->get_accepted())
    {
        std::cout << "failed to add server: " << ret->get_result_code() << std::endl;
        return -1;
    }
    std::cout << "async request is in progress (check with `serverlist` command)" << std::endl;

    return 0;
}

int HttpServiceImpl::RemoveServer(int server_id)
{
    if (server_id < 1 ||  server_id == ctx_->server_id_)
    {
        std::cout << "wrong server id: " << server_id << std::endl;
        return -1;
    }

    nuraft::ptr<raft_result> ret = ctx_->raft_instance_->remove_srv(server_id);
    if (!ret->get_accepted())
    {
        std::cout << "failed to add server: " << ret->get_result_code() << std::endl;
        return -1;
    }
    std::cout << "async request is in progress (check with `serverlist` command)" << std::endl;

    return 0;
}

    int HttpServiceImpl::GetQuotes(const std::vector<std::string>& codes, std::vector<CerebroTickRecordPtr>& records)
    {
        // force to update quotes
        //mdlink_.update_quotes();
        for(auto const& c: codes)
        {
            mdlink_.update_quotes(c);
        }

        LOGD("update_quotes codes.size=%lu", codes.size());
        {
        util::RWMutex::ReadLock lck(quotes_mtx_);
        for(auto const& s: codes)
        {
            auto c = mdlink_api::MarketDataBasic::convert_to_extend_code(s);
            auto it = quotes_.find(c);
            if(it == quotes_.cend())
            {
                continue;
            }
            records.push_back(it->second);
        }
        }
        return 0;
    }
    int HttpServiceImpl::SubscribeQuotes(const std::vector<std::string>& codes)
    {
    mdlink_.set_codes(codes);
    return 0;
    }
    CerebroTraderPtr HttpServiceImpl::CreateTrader(const std::string& name, double cash)
    {
        do{
            util::RWMutex::ReadLock lck(traders_mtx_);
            auto it = traders_nm_.find(name);
            if(it == traders_nm_.cend())
            {
                break;
            }
            auto trader = traders_.at(it->second);
            return trader;
        }while(0);

        auto id = ctx_->trader_id_gen.nextid();
        auto trader = std::make_shared<CerebroTrader>();
        // 必先初始化前设置order_id_generator
        trader->get_broker()->set_order_id_generator([this]()->int64_t{
            return ctx_->order_id_gen.nextid();
        });
        trader->init(id, name, cash);

        util::RWMutex::WriteLock lck(traders_mtx_);
        traders_[id] = trader;
        traders_nm_[name] = id;
        return trader;
    }
    int HttpServiceImpl::DeleteTrader(int64_t id)
    {
        util::RWMutex::WriteLock lck(traders_mtx_);
        auto it = traders_.find(id);
        if(it == traders_.cend())
        {
            return -1;
        }
        else 
        {
            traders_nm_.erase(it->second->name());
            traders_.erase(id);
        }
        return 0;
    }
    CerebroTraderPtr HttpServiceImpl::GetTrader(int64_t id)
    {
        util::RWMutex::ReadLock lck(traders_mtx_);
        auto it = traders_.find(id);
        if(it != traders_.cend())
        {
            return it->second;
        }
        return nullptr;
    }

    CerebroTraderPtr HttpServiceImpl::GetTrader(const std::string& name)
    {
        util::RWMutex::ReadLock lck(traders_mtx_);
        auto nm = traders_nm_.find(name);
        if(nm == traders_nm_.cend())
        {
            return nullptr;
        }

        auto it = traders_.find(nm->second);
        if(it != traders_.cend())
        {
            return it->second;
        }
        return nullptr;
    }

    int64_t HttpServiceImpl::EnterOrder(const TradeOrderRequest& req)
    {
        auto trader = GetTrader(req.trader_id);
        if(trader == nullptr)
        {
            LOGE("invalid trader id:%lu", req.trader_id);
            return -1;
        }
        int64_t id = 0;
        if(req.side == 1)
        {
        id = trader->get_broker()->buy(req.symbol, req.quantity, req.price);
        }
        else if(req.side == 2) {
        id = trader->get_broker()->sell(req.symbol, req.quantity, req.price);
        }
        else
        {
            LOGE("invalid order side:%d, trader_id:%lu",  req.side, req.trader_id);
            return -1;
        }
        if(id < 1)
        {
            LOGE("enter order commit failed, trader_id:%lu, symbol:%s",  req.trader_id, req.symbol.c_str());
        }
        return id;
    }
    int64_t HttpServiceImpl::CancelOrder(int64_t trader_id, int64_t order_id)
    {
        auto trader = GetTrader(trader_id);
        if(trader == nullptr)
        {
            LOGE("invalid trader id:%lu", trader_id);
            return -1;
        }
        int64_t id = trader->get_broker()->cancel_order(order_id);
        if(id < 1)
        {
            LOGE("cancel order commit failed, trader_id:%lu, order_id:%lu",  trader_id, order_id);
        }
        return id;
    }
    int HttpServiceImpl::OrderList(int64_t trader_id, std::vector<CerebroOrder>& orders)
    {
        auto trader = GetTrader(trader_id);
        if(trader == nullptr)
        {
            LOGE("invalid trader id:%lu", trader_id);
            return -1;
        }
        return  trader->get_broker()->current_orders(orders);
    }
    int HttpServiceImpl::PositionList(int64_t trader_id, std::vector<CerebroPosition>& positions)
    {
        auto trader = GetTrader(trader_id);
        if(trader == nullptr)
        {
            LOGE("invalid trader id:%lu", trader_id);
            return -1;
        }
        return  trader->get_broker()->current_positions(positions);
    }


void HttpServiceImpl::append_log(aquadb::BinlogRecord *record)
{
    // TODO
}
