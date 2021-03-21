#pragma once
#include <absl/time/time.h>
#include "util/common.h"
#include "util/logdef.h"
#include "proto/security.pb.h"
#include "ordermanager.h"

// 投资组合-策略所有账户的集合
class CerebroPortfolio
{
public:
    // accounts
    // TODO

};

// 账户-多种持仓和现金的集合
class CerebroAccount
{
public:
    //int64_t id;
    // int dt;
    // 总保证金
    double  margin;
    // 多方向保证金
    double  buy_margin;
    // 空方向保证金
    double  sell_margin;
    // 可用资金
    double  cash;
    // 账户总资金
    double  total_cash;
    // 当日盈亏
    double  daily_pnl;
    // 总权益
    double  equity;
    // 冻结资金
    double  frozen_cash;
    // 市值
    double  market_value;
    // 昨仓盈亏
    double  position_pnl;
    // 账户总权益
    double  total_value;
    // 交易盈亏
    double  trading_pnl;
    // 总费用
    double  transaction_cost;

    // 根据计算当前不超过 trade_amount 的最大可平仓量
    int calc_close_today_amount();
};

// 持仓方向
enum class POSITION_DIRECTION
{
    None = 0,
    // 多方向
    LONG = 1,
    // 空方向
    SHORT =2
};

// 交易动作
enum class POSITION_EFFECT
{
    None = 0,
    // 开仓
    OPEN = 1,
    // 平仓
    CLOSE = 2,
    // 平今
    CLOSE_TODAY = 3,
    // 行权
    EXERCISE = 4,
    // 轧差
    MATCH =5
};

// 订单方向
enum class ORDER_SIDE
{
    None = 0,
    // 买
    BUY = 1,
    // 卖
    SELL =2
};

// 权利类型
enum class RIGHT_TYPE
{
    None = 0,
    CONVERT = 1,
    SELL_BACK = 2
};

// 订单类型
enum class ORDER_TYPE
{
    None = 0,
    MARKET = 1,
    LIMIT = 2
};

// 订单状态
enum class ORDER_STATUS
{
   None = 0,
    // 待报
     PENDING_NEW = 1,
    // 已报
    ACTIVE = 2,
    // 成交
    FILLED = 3,
    // 已撤
    CANCELLED = 4,
    // 拒单
    REJECTED = 5
};

class CerebroOrder
{
public:
    // 成交均价
    double filled_price;

    // 创建时间
    long long ctime;
    // 修改时间
    long long mtime;
    // 已成交量
    double filled_quantity;
    // 未成交量
    double unfilled_quantity;
    // 冻结价格
    double frozen_price;
    // 消息
    std::string message;
    // 合约
    Symbol symbol;
    // 订单id
    long long order_id;
    // 母单id
    //long long base_order_id;

    // 订单开平
    POSITION_EFFECT position_effect;

    // 订单价格
    double price;

    // 订单量
    double quantity;

    // 扩展字段
    std::map<std::string,std::string> extra;

    // 订单方向
    ORDER_SIDE side;
    // 状态
    ORDER_STATUS status;
    // 订单类型
    ORDER_TYPE type;

    // 订单日期
    int trading_date;

    // 费用
    double transaction_cost;
};

// 持仓
class CerebroPosition
{
public:
    // 可平仓位
    double  closable;
    // 当前持仓的方向
    POSITION_DIRECTION direction;
    // 当前持仓所占的保证金
    double margin;
    // 当前持仓的市值
    double market_value;
    // 当前持仓的标的
    Symbol symbol;
    // 持仓累积盈亏
    double pnl;
    // 当前持仓当日持仓盈亏
    double position_pnl;
    // 当前持仓量
    double quantity;
    // 今仓中的可平仓位
    double  today_closable;
    // 当前持仓当日的交易盈亏
    double trading_pnl;
};


// 交易标的
class CerebroInstrument
{
public:
    // 标的
    Symbol symbol;
    // 交易所证券代码
    std::string code;
    // 证券名称
    std::string name;
    // 最小价格变动单位
    double tick_size;
    // 股票：一手对应多少股，中国A股一手是100股。期货：一律为1
    int round_lot;
    // 板块类别
    int board_type;
    // 股票：该证券上市日期。期货：期货的上市日期
    int listed_date;
    // 股票：退市日期。期货：交割日期。
    int de_listed_date;
    // 合约状态。'Active' - 正常上市, 'Delisted' - 终止上市, 'TemporarySuspended' - 暂停上市, 'PreIPO' - 发行配售期间, 'FailIPO' - 发行失败
    int status;
    // 合约乘数，例如沪深300股指期货的乘数为300.0
    double contract_multiplier;
    // 期货到期日
    int maturity_date;
    // 产品类型，'Index' - 股指期货, 'Commodity' - 商品期货, 'Government' - 国债期货
    int product;
};

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

