#include "cerebro_trader.h"
#include "util/common.h"

  CerebroTrader::CerebroTrader() 
  {
    broker_ = new CerebroBroker();

  }
    CerebroTrader::~CerebroTrader()
    {
        delete broker_;
    }

int CerebroTrader::init(int64_t id, const std::string& name, double cash)
{
    id_ = id;
    name_ = name;
    // 此处账户Id和交易Id保持一样
    account_ = std::make_shared<CerebroAccountWrap>(id, name);
    broker_->init(account_);
    // 入初始资金
    if(state_ == 0) {
        broker_->deposit_cash(cash);
    }

    broker_->set_matcher("SH", new CerebroSSEMatcher(broker_));
    broker_->set_matcher("SZ", new CerebroSZSEMatcher(broker_));
    return 0;
}
