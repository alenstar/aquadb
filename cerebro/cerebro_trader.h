#pragma once

#include "cerebro_struct.h"
#include "cerebro_broker.h"
#include "nlohmann/json.hpp"

class CerebroTrader
{
    public:
    CerebroTrader() ;
    ~CerebroTrader();

    int init(int64_t id, const std::string& name, double cash);
    inline int64_t id() const { return id_;}
    inline const std::string& name() const { return name_;}

    inline CerebroAccountWrapPtr get_account() { return account_;}
    inline CerebroBroker* get_broker() { return broker_;}

    nlohmann::json to_json() const {
        nlohmann::json js;
        js["id"] = id_;
        js["name"] = name_;
        js["state"] = state_;
        if(account_)
        {
            js["account"] = account_->to_json();
        }
        return js;
    }
        std::string to_json_string() const {
      auto js = to_json();
      return js.dump();
    }
    private:
    int64_t id_;
    std::string name_;
    CerebroAccountWrapPtr account_{nullptr};
    CerebroBroker* broker_{nullptr};
    int state_ {0};
};
typedef std::shared_ptr<CerebroTrader> CerebroTraderPtr;