#pragma once
#include "cerebro_struct.h"
#include "cerebro_quote.h"

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
    // 行情数据驱动
    int run(CerebroQuoteProvider* provider);

    //添加策略， 用户自己管理策略对象
    int add_strategy(CerebroStrategy* strategy) { if(strategys_.find(strategy->name()) != strategys_.cend()) {return -1;} strategys_[strategy->name()] = strategy; return 0; }
    //添加分析器， 用户自己管理分析器对象 
    int add_analyzer(CerebroAnalyzer* analyzer) { if(analyzers_.find(analyzer->name()) != analyzers_.cend()) {return -1;} analyzers_[analyzer->name()] = analyzer; return 0; }

    nlohmann::json to_json() const;
    std::string to_json_string() const
    {
        auto js = to_json();
        return js.dump();
    }

  protected:
  private:
    CerebroConfig conf_;
    std::map<std::string, CerebroStrategy*> strategys_;
    std::map<std::string, CerebroAnalyzer*> analyzers_;
    CerebroBroker *broker_{nullptr};
    std::shared_ptr<CerebroAccountWrap> account_{nullptr};
    //std::shared_ptr<CerebroQuoteProvider> qprovider_{nullptr};
    int64_t last_order_id_{0};
    int state_ {0};
};