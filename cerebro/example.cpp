#include <iostream>
#include <chrono>
#include "cerebro_struct.h"
#include "cerebro_broker.h"
#include "cerebro_trader.h"
#include "cerebro_backtest.h"
#include "sina_api.h"
#include "tencent_api.h"
INITIALIZE_EASYLOGGINGPP

class MyStrategy:public CerebroStrategy
{
    public:
    MyStrategy () {name_= "MyStrategy";}
    ~MyStrategy() override {}
    void on_market_open(CerebroBroker* broker, int dt) override {
        LOGD("dt=%d",dt);
    }
    void on_pre_market_open(CerebroBroker* broker, int dt) override {
        LOGD("dt=%d",dt);
    }
    void on_market_close(CerebroBroker* broker, int dt) override {
        LOGD("dt=%d",dt);
    }
    void on_aft_market_close(CerebroBroker* broker, int dt) override {
        LOGD("dt=%d",dt);
    }
    void on_tick(CerebroBroker* broker, CerebroTickRecord* tick) override   // 加载tick缓存， 触发用户业务回调on_tick，再触发broker撮合
    {
        LOGD("symbol=%s", tick->symbol.c_str());
    }
    void on_kline(CerebroBroker* broker, CerebroKlineRecord* kline) override // 根据时间点加载k线缓存，再处理用户业务回调on_kline
    {
        LOGD("symbol=%s", kline->symbol.c_str());
    }
    void on_order_update(CerebroBroker* broker,const CerebroOrder* order) override {
        LOGD("symbol=%s,%f,%f, %d", order->symbol.c_str(), order->price, order->quantity, static_cast<int>(order->status));
    }

};

int main(int argc ,char* argv[])
{
    int rc =0 ;
    
    std::vector<std::string> codes;//{"sz000001","sh600001","sh688681","sh688680"};
    mdlink_api::MarketDataProvider mdlink;
    mdlink.init(8);
    rc = mdlink.update_codes();
    if(rc != 0)
    {
        std::cerr << "get_codes failed" << std::endl;
    }
    std::cout << "codes.size=" << mdlink.get_codes().size() << std::endl;
    mdlink.update_quotes();
    // mdlink.wait_for_shutdown();

    // backtesting
    // TODO
    CerebroConfig config;
    config.cash = 1000000;
    config.qtype = QUOTES_TYPE::DAY;
    config.rtype = STRATEGY_RUN_TYPE::BACKTEST;
    config.comm_rate = 0.0003;
    config.slippage = 1;
    config.start_date = 20210101;
    config.end_date = 20210801;
    CerebroBacktest bt;
    rc = bt.init(config);
    if(rc != 0)
    {
        LOGE("backtest init fail, rc=%d", rc);
        return 1;
    }
    auto strategy = std::make_shared<MyStrategy>();
    bt.add_strategy(strategy);
    rc = bt.run();
    if(rc != 0)
    {
        LOGE("backtest reutrn_code: %d", rc);
        return 2;
    }
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}
