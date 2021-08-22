#pragma once
#include "cerebro_struct.h"
#include "cerebro_recorder.h"

class CerebroBroker;
class CerebroAccountWrap;
class CerebroQuoteProvider;
class CerebroBacktest
{
  public:
    CerebroBacktest();
    ~CerebroBacktest();
    //void set_quote_provider(std::shared_ptr<CerebroQuoteProvider> provider);
    int init(const CerebroConfig &conf);
    int run();

    void add_strategy(CerebroStrategyPtr strategy) { strategys_[strategy->name()] = strategy; }

    nlohmann::json to_json() const;
    std::string to_json_string() const
    {
        auto js = to_json();
        return js.dump();
    }

  protected:
  private:
    CerebroConfig conf_;
    std::map<std::string, CerebroStrategyPtr> strategys_;
    CerebroBroker *broker_{nullptr};
    std::shared_ptr<CerebroAccountWrap> account_{nullptr};
    //std::shared_ptr<CerebroQuoteProvider> qprovider_{nullptr};
    int64_t last_order_id_{0};
    int state_ {0};
};