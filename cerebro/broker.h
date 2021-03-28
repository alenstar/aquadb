#pragma once
#include "util/common.h"
#include "util/logdef.h"
#include "cerebro_struct.h"

class BrokerBase
{
public:
    BrokerBase();
    virtual ~BrokerBase();

    int init();

    void add_cash(double cash){

    }

    void set_cash(double cash){
        _cash = cash;
    }

    double get_cash() const {
        return _cash;
    }

    int64_t buy(const std::string& symbol, int64_t qty, double price);
    int64_t sell(const std::string& symbol, int64_t qty, double price);

    int64_t buy_open(const std::string& symbol, int64_t qty, double price);
    int64_t sell_open(const std::string& symbol, int64_t qty, double price);
    int64_t buy_close(const std::string& symbol, int64_t qty, double price);
    int64_t sell_close(const std::string& symbol, int64_t qty, double price);

    int64_t cancel_order(int64_t order_id);

    void match_order_book(const std::string& symbol, const KBar& bar){

    }

    void match_order_book(const std::string& symbol, double price){
        match(symbol, price, _timestamp_ms);
    }

    void match_order_book(const std::string& symbol, const sofa::pbrpc::quant::TickRecord& bar){
    }

    void match_order_book(const std::string& symbol, const sofa::pbrpc::quant::KLineRecord& bar){
        _timestamp_ms = bar.ts();
        sofa::pbrpc::quant::TickRecord tick;
        tick.set_ts(bar.ts());
        tick.set_last(bar.close());
        tick.set_open(bar.open());
        tick.set_low(bar.low());
        tick.set_high(bar.high());
        tick.set_amount(bar.amount());
        tick.set_volumn(bar.volumn());
        match(symbol, tick);
    }

    void match(const std::string& symbol, double price, int64_t ts);
    void match(const std::string& symbol, const sofa::pbrpc::quant::TickRecord& bar);

    //分红送股时做 除权除息
    // 根据昨收价判断是否发生除权除熙
    // 处理方式: 现金分红(增加对应比例的资金), 送股(增加对应比例的持仓)
    void adjust();

    void update_filled_position(const std::string& symbol, int64_t qty, double price, int64_t ts, bool is_buy){
    auto p = get_position(symbol,true, true);
    //LOG_INFO("symbol: " << symbol << " "<< qty << " " << price << " " << ts);
    p->fill(qty, price, is_buy);
    }
    PositionPtr get_position(const std::string& symbol, bool is_long = true, bool auto_create = false);
    void update_position(){}

    void update_tick(const std::string& symbol, const sofa::pbrpc::quant::TickRecord& bar){}
    void update_timestamp_ms(int64_t ts){
        _timestamp_ms = ts;
    }

    int64_t get_timestamp_ms(){
        // 回测用broker的时间戳根据k线或tick更新
        return _timestamp_ms;
    }

    OrderExecutorPtr new_order(Order& o, int64_t timestamp = 0) {
        o.id = OrderUniqueSequence::get();
        o.status = OrderStatus::NEW;
        o.ctime = timestamp;
        return std::make_shared<OrderExecutor>(o);
    }

    int append_order(const Order& o) {
        auto oe = std::make_shared<OrderExecutor>(o);
        _orders[o.symbol].emplace_back(oe);
        return 0;
    }



    absl::Time get_current_time() const;
    int get_current_date() const;

    void set_trading_schedule(const std::vector<std::pair<absl::Time, absl::Time>>& days);
    void set_holidays(const std::vector<int>& days);

    int get_prev_trading_date(int dt, int offset) const;
    int get_next_trading_day(int dt, int offset) const;

    int get_prev_trading_date(int dt, std::vector<int>& days) const;
    int get_next_trading_date(int dt, std::vector<int>& days) const;

    int get_trading_date(int start_dt, int end_dt, std::vector<int>& days) const;

    int get_trading_timeline(int dt, std::vector<absl::Time>& line) const;
protected:
    double _cash;
    int _slippage;
    uint64_t _user_id;
    // 用户持仓管理
    // 一个用户一个broker实例
    // 持仓管理
    PositionMgrPtr _pmgr;
    OrderMgrPtr _omgr;
    int64_t _timestamp_ms;

    // Trading schedule
    // <open trading time, close trading time>
    std::vector<std::pair<absl::Time, absl::Time>> _trading_schedule;

    std::unordered_map<std::string, std::vector<OrderExecutorPtr>> _orders;
    std::unordered_map<std::string, std::vector<OrderExecutorPtr>> _ordersfilled;
};
typedef std::shared_ptr<BrokerBase> BrokerBasePtr;

