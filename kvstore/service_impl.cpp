#include "nlohmann/json.hpp"
#include "service_impl.h"
#include "db/db.h"

#include "state_machine.h"
#include "state_mgr.h"

#define GET_STATE_MACHINE(x) (static_cast<nuraft::rocksdb_state_machine *>(x->sm_.get()))

using raft_result = nuraft::cmd_result<nuraft::ptr<nuraft::buffer>>;

int HttpServiceImpl::Init(GlobalContext *ctx)
{
    ctx_ = ctx;

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
            if(out.empty())
            {
                js["data"] = nullptr;
            }
            else {
            js["data"] = nlohmann::json::array();
            for(auto& c: out)
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
    out.server_id = ctx_->server_id_ ;
out.leader_id = ctx_->raft_instance_->get_leader();
       out.log_start_index = ls->start_index();
      out.log_end_index =  ls->next_slot() - 1;
       out.last_committed_index = ctx_->raft_instance_->get_committed_log_idx();
       // "state machine value: " << GET_STATE_MACHINE(ctx_)->get_current_value() << "\n";
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
    std::cout << "async request is in progress (check with `list` command)" << std::endl;

    return -1;
}
int HttpServiceImpl::ServerList(std::vector<ServerRaftConfig>& out)
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
        cfg.aux= srv->get_aux();
        cfg.learner= srv->is_learner();
        if (srv->get_id() == leader_id)
        {
        cfg.leader =  true;
        }
        else {
        cfg.leader =  false;
        }
        cfg.priority = srv->get_priority();
        out.emplace_back(cfg);
    }
    return 0;
}

void HttpServiceImpl::append_log(aquadb::BinlogRecord *record)
{
    // TODO
}
