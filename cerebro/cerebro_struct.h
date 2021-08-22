#pragma once
#include <cstdint>
#include <memory>
#include "util/common.h"
#include "util/logdef.h"
#include "nlohmann/json.hpp"

typedef std::string Symbol;

class CerebroBroker;
class CerebroDataFeed;
// 投资组合-策略所有账户的集合
class CerebroPortfolio
{
  public:
    // accounts // 账户字典
    // TODO
    double cash = 0;               // 可用资金
    double daily_pnl = 0;          // 当日盈亏
    double annualized_returns = 0; // 累计年化收益率
    double daily_returns = 0;      // 当前最新一天的日收益

    double frozen_cash = 0;           // 冻结资金
    double market_value = 0;          // 市值
    double pnl = 0;                   // 收益
    double portfolio_value = 0;       // 总权益
    int start_date = 0;               // 策略投资组合的开始日期
    double starting_cash = 0;         // 初始资金
    double static_unit_net_value = 0; // 昨日净值

    double total_returns = 0;    //
    double total_value = 0;      //
    double transaction_cost = 0; // 交易成本（税费）
    double unit_net_value = 0;   // 实时净值, 根据净值计算累积收益
    double units = 0;            // 份额 
};

// 账户-多种持仓和现金的集合
class CerebroAccount
{
  public:
    // int64_t id = 0;
    // int dt = 0;
    // 总保证金
    double margin = 0;
    // 多方向保证金
    double buy_margin = 0;
    // 空方向保证金
    double sell_margin = 0;
    // 可用资金
    double cash = 0;
    // 账户总资金 (总资产=可用资金+冻结资金)
    // double total_cash = 0;
    // 当日盈亏(根据行情变化来更新，结算后清零)
    double daily_pnl = 0;
    // 总权益
    double equity = 0;
    // 冻结资金
    double frozen_cash = 0;
    // 市值 (交易导致市值变化)
    double market_value = 0;
    // 持仓盈亏 (结算时更新)
    double position_pnl = 0;
    // 账户总权益
    // double total_value = 0;
    // 交易盈亏 (平仓时计算)
    double trading_pnl = 0;
    // 总费用
    double transaction_cost = 0;

    // 份额
    double units = 0; // 初次入金时， 值为 total_cash; 后面中途入金 份额 = (units*unit_value + into_cash)/单位价值
    // 单位价值
    double unit_value = 0;  // 初次入金时， 值为1

    // 根据计算当前不超过 trade_amount 的最大可平仓量
    int calc_close_today_amount();

    // 账户总资金 (总资产=可用资金+冻结资金+市值)
    inline double total_cash() const { return cash + frozen_cash;}
    // 账户总权益
    inline double total_value() const {return total_cash() + market_value;}
    // 当前市值
    inline double current_market_value() const {return market_value + daily_pnl;}

    nlohmann::json to_json() const {
      nlohmann::json js;
      js["margin"] = margin;
      js["buy_margin"] = buy_margin;
      js["sell_margin"] = sell_margin;
      js["cash"] = cash;
      js["total_cash"] = total_cash();
      js["daily_pnl"] = daily_pnl;
      js["equity"] = equity;
      js["frozen_cash"] = frozen_cash;
      js["market_value"] = market_value;
      js["position_pnl"] = position_pnl;
      js["total_value"] = total_value();
      js["trading_pnl"] = trading_pnl;
      js["transaction_cost"] = transaction_cost;
      js["units"] = units;
      js["unit_value"] = unit_value;
      return js;
    }
    std::string to_json_string() const {
      auto js = to_json();
      return js.dump();
    }
};

// 持仓方向
enum class POSITION_DIRECTION
{
    NONE = 0,
    // 多方向
    LONG = 1,
    // 空方向
    SHORT = 2
};

// 交易动作
enum class POSITION_EFFECT
{
    NONE = 0,
    // 开仓
    OPEN = 1,
    // 平仓
    CLOSE = 2,
    // 平今
    CLOSE_TODAY = 3,
    // 行权
    EXERCISE = 4,
    // 轧差
    MATCH = 5
};

// 订单方向
enum class ORDER_ACTION
{
    NONE = 0,
    // 买开
    BUY = 1,
    // 卖平
    SELL = 2,
    // 买平
    BUY_TO_COVER = 3,
    // 卖空/卖开
    SELL_SHORT = 4, 

    // 撤单
    // 撤单视作特殊的订单类型
    CANCEL = 11, 
    // 改单
    AMEND = 12,

    // 分红送股可以视作特殊的订单类型, 这样来保证账户资产变动的连续性
    DIVIDEND = 101, // 分红
    // 入金
    DEPOSIT = 103,
    // 出金
    WITHDRAW = 104

};

// 权利类型
enum class RIGHT_TYPE
{
    NONE = 0,
    CONVERT = 1,
    SELL_BACK = 2
};

// 订单类型
enum class ORDER_TYPE
{
    NONE = 0,
    MARKET = 1,
    LIMIT = 2

};

// 订单状态
enum class ORDER_STATUS
{
    NONE = 0,
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

// 行情类型
enum class QUOTES_TYPE { TICK = 0, MIN1 = 60, MIN5 = 60 * 5, MIN15 = 60 * 15, DAY = 60 * 60 * 24 };
enum class STRATEGY_RUN_TYPE { NONE = 0, BACKTEST = 1, SIMULATE = 2, ACTUAL = 3 };

class CerebroOrder
{
  public:
    // 成交均价
    double filled_price = 0;

    // 创建时间
    long long ctime = 0;
    // 修改时间
    long long mtime = 0;
    // 已成交量
    double filled_quantity = 0;
    // 未成交量
    double unfilled_quantity = 0;
    // 冻结价格
    double frozen_price = 0;
    // 消息 (下单是填充)
    std::string msg;
    // 原因 (撮合时，系统填充, 用于描述未成交原因)
    std::string reason;
    // 合约
    Symbol symbol;
    // 订单id
    long long order_id = 0;
    // 母单id
    // long long base_order_id = 0;

    // 订单开平
    // POSITION_EFFECT position_effect;

    // 订单价格
    double price = 0;

    // 订单量
    double quantity = 0;

    // 扩展字段
    std::map<std::string, std::string> extra; // base_oid 母单order_id 部分成交时子单填写母单order_id; 

    // 订单动作/方向
    ORDER_ACTION action = ORDER_ACTION::NONE;
    // 状态
    ORDER_STATUS status = ORDER_STATUS::NONE;
    // 订单类型
    ORDER_TYPE type = ORDER_TYPE::NONE;

    // 订单日期
    int trading_date = 0;

    // 费用
    double transaction_cost = 0;

    nlohmann::json to_json() const {
      nlohmann::json js;
      js["filled_price"] = filled_price;
      js["ctime"] = ctime;
      js["mtime"] = mtime;
      js["filled_quantity"] = filled_quantity;
      js["unfilled_quantity"] = unfilled_quantity;
      js["frozen_price"] = frozen_price;
      js["symbol"] = symbol;
      js["order_id"] = order_id;
      js["price"] = price;
      js["quantity"] = quantity;
      js["action"] = action;
      js["status"] = status;
      js["type"] = type;
      js["trading_date"] = trading_date;
      js["transaction_cost"] = transaction_cost;
      js["reason"] = reason;
      js["msg"] = msg;
      return js;
    }
    std::string to_json_string() const {
      auto js = to_json();
      return js.dump();
    }

            static nlohmann::json to_json(const std::vector<CerebroOrder>& poss) {
        nlohmann::json::array_t arr;
        for(auto const& p: poss)
        {
          arr.emplace_back(p.to_json());
        }
        return arr;
        }
};

// 持仓
class CerebroPosition
{
  public:
    // 可平仓位
    double closable = 0;
    // 当前持仓的方向
    POSITION_DIRECTION direction = POSITION_DIRECTION::NONE;
    // 当前持仓所占的保证金
    double margin = 0;
    // 当前持仓的市值
    //double market_value = 0;
    // 当前持仓的标的
    Symbol symbol;
    // 持仓累积盈亏
    double pnl = 0;
    // 当前持仓盈亏
    double position_pnl = 0;
    // 当前持仓量
    double quantity = 0;
    // 今仓中的可平仓位
    double today_closable = 0;
    // 持仓交易盈亏(平仓时计算)
    double trading_pnl = 0;

    // 持仓均价
    double avg_price = 0.0;
    // 总手续费
    double commissions = 0.0;

    // 持仓价值 avg_price * quantity

    // xxxx 
    double total_commited = 0.0;

    // 当前持仓的市值
    inline double market_value() const {
      return avg_price * quantity;
    }
    
    nlohmann::json to_json() const {
      nlohmann::json js;
      js["closable"] = closable;
      js["direction"] = direction;
      js["margin"] = margin;
      js["market_value"] = market_value();
      js["symbol"] = symbol;
      js["pnl"] = pnl;
      js["position_pnl"] = position_pnl;
      js["quantity"] = quantity;
      js["today_closable"] = today_closable;
      js["trading_pnl"] = trading_pnl;
      js["avg_price"] = avg_price;
      js["commissions"] = commissions;
      return js;
    }

    std::string to_json_string() const {
      auto js = to_json();
      return js.dump();
    }

        static nlohmann::json to_json(const std::vector<CerebroPosition>& poss) {
        nlohmann::json::array_t arr;
        for(auto const& p: poss)
        {
          arr.emplace_back(p.to_json());
        }
        return arr;
        }

};

// 交易品种
class CerebroProduct
{
  public:
    std::string code; // 商品代码 AP，br，bu 等; 特殊代码 CS: Common Stock, 即股票;  ETF: Exchange Traded Fund,
                      // 即交易所交易基金; LOF Listed Open-Ended Fund，即上市型开放式基金; INDX: Index, 即指数; FT:
                      // Futures，即期货，包含股指、国债和商品期货; OP: Option 期权
    // 产品类型，'Index' - 股指期货, 'Commodity' - 商品期货, 'Government' - 国债期货;
    int product;
    std::string name; //
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
    // 交易所
    std::string exchange; // SZ,SH,HK, DCE,CZCE,SHFE,CFFEX,INE,SGE, OF(开放式基金交易所使用OF)
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
    // 合约状态。'Active' - 正常上市, 'Delisted' - 终止上市, 'TemporarySuspended' - 暂停上市, 'PreIPO' - 发行配售期间,
    // 'FailIPO' - 发行失败
    int status;
    // 合约乘数，例如沪深300股指期货的乘数为300.0
    double contract_multiplier;
    // 期货到期日
    int maturity_date;
    // 产品类型，'Index' - 股指期货, 'Commodity' - 商品期货, 'Government' - 国债期货
    int product;
    int min_volume;         // 最小委托量
    int max_volume;         // 最大委托量
    int long_margin_rate;   // 多头保证金率
    int short_margin_rate;  // 空头保证金率
    double commission_rate; // 手续费率
};

class CerebroTickRecord
{
  public:
    Symbol symbol; // 标的
    int64_t ts = 0;    // 当前快照数据的时间戳(毫秒)
    int32_t dt = 0;        // 交易日 YYYYMMDD
    //int32_t time; // 时间 HHMMSS

    std::vector<double> ask_vols; // 卖出报盘数量，ask_vols[0]代表盘口卖一档报盘数量
    std::vector<double> asks;     // 卖出报盘价格，asks[0]代表盘口卖一档报盘价
    std::vector<double> bid_vols; // 买入报盘数量，bids_vols[0]代表盘口买一档报盘数量
    std::vector<double> bids;     // 买入报盘价格，bids[0]代表盘口买一档报盘价
    double high = 0;                  // 截止到当前的最高价
    double last = 0;                  // 当前最新价
    double low = 0;                   // 截止到当前的最低价
    double open = 0;                  // 当日开盘价
    double prev_close = 0;            // 昨日收盘价
    double turnover = 0;        // 截止到当前的成交额
    double volume = 0;                // 截止到当前的成交量

    double open_interest = 0;      // 截止到当前的持仓量（期货专用）
    double prev_open_interest = 0; // 昨日的持仓量（期货专用）
    double settlement = 0;         // 结算价（期货专用）
    double prev_settlement = 0;    // 昨日结算价（期货专用）

    double limit_down = 0; // 跌停价
    double limit_up = 0;   // 涨停价

    friend std::ostream &operator<<(std::ostream &oss, const CerebroTickRecord &o) 
    {
            oss << o.symbol <<",";
            oss << o.dt <<",";
            oss << o.ts <<",";
            oss << o.open <<",";
            oss << o.high <<",";
            oss << o.low <<",";
            //oss << o.close <<",";
            oss << o.last <<",";
          
            oss << o.prev_close << ",";
            oss << o.volume << ",";
            for(size_t i = 0; i < 5;++i)
            {
                oss << o.bids[i] << ",";
                oss << o.bid_vols[i] << ",";
                oss << o.asks[i] << ",";
                oss << o.ask_vols[i] << ",";
            }
              oss << o.limit_up <<",";
            oss << o.limit_down;
            return oss;
    }
    
    nlohmann::json to_json() const {
      nlohmann::json js;
      js["ts"] = ts;
      js["dt"] = dt;
      js["symbol"] = symbol;
      js["last"] = last;
      js["high"] = high;
      js["low"] = low;
      js["open"] = open;
      //js["close"] = close;
      js["prev_close"] = prev_close;
      js["turnover"] = turnover;
      js["volume"] = volume;
      js["bids"] = nlohmann::json::array_t();
      for(auto a: bids)
      {
        js["bids"].push_back(a);
      }
      js["bid_vols"] = nlohmann::json::array_t();
            for(auto a: bid_vols)
      {
        js["bid_vols"].push_back(a);
      }
      js["asks"] = nlohmann::json::array_t();
            for(auto a: asks)
      {
        js["asks"].push_back(a);
      }
      js["ask_vols"] = nlohmann::json::array_t();
            for(auto a: ask_vols)
      {
        js["ask_vols"].push_back(a);
      }

      js["limit_up"] = limit_up;
      js["limit_down"] = limit_down;
      return js;
    }
    std::string to_json_string() const {
      auto js = to_json();
      return js.dump();
    }
    static nlohmann::json to_json(const std::vector<CerebroTickRecord>& records) {
      nlohmann::json::array_t js;
      for(auto const& r: records)
      {
        js.emplace_back(r.to_json());
      }
      return js;
    }
};
typedef std::shared_ptr<CerebroTickRecord> CerebroTickRecordPtr;

class CerebroKlineRecord
{
  public:
    Symbol symbol;         // 标的
    int64_t ts = 0;            // 当前快照数据的时间戳
    int dt = 0;
    //int type;              // k线类型 1m 5m 1d
    double high = 0;           // 截止到当前的最高价
    double close = 0;          // 当前最新价
    double low = 0;            // 截止到当前的最低价
    double open = 0;           // 当日开盘价
    double prev_close = 0;     // 昨日收盘价
    double turnover = 0; // 截止到当前的成交额
    double volume = 0;         // 截止到当前的成交

    double open_interest = 0;      // 截止到当前的持仓量（期货专用）
    double prev_open_interest = 0; // 昨日的持仓量（期货专用）
    double prev_settlement = 0;    // 昨日结算价（期货专用）
    double settlement = 0;         // 结算价（期货专用）


    nlohmann::json to_json() const {
      nlohmann::json js;
      js["ts"] = ts;
      js["dt"] = dt;
      js["symbol"] = symbol;
      //js["type"] = type;
      js["high"] = high;
      js["low"] = low;
      js["open"] = open;
      js["close"] = close;
      js["prev_close"] = prev_close;
      js["turnover"] = turnover;
      js["volume"] = volume;
        js["open_interest"] = open_interest;
        js["prev_open_interest"] = prev_open_interest;
        js["prev_settlement"] = prev_settlement;
        js["settlement"] = settlement;

      //js["limit_up"] = limit_up;
      //js["limit_down"] = limit_down;
      return js;
    }
        std::string to_json_string() const {
      auto js = to_json();
      return js.dump();
    }
    static nlohmann::json to_json(const std::vector<CerebroKlineRecord>& records) {
      nlohmann::json::array_t js;
      for(auto const& r: records)
      {
        js.emplace_back(r.to_json());
      }
      return js;
    }
};

class CerebroTime;

// 日历对象
class CerebroCalendar
{
  public:
    CerebroCalendar();
    bool is_trading(CerebroTime *t);

  private:
    std::vector<int> _days;
    std::vector<std::pair<int, int>> _time_ranges; // 交易时间范围
};

// 时间对象
class CerebroTime final
{
  public:
    CerebroTime(int dt);
    CerebroTime(const CerebroTime &ct);
    int64_t ts() const;
    int time() const; // 返回时间对象
    CerebroTime &add_seconds(int n);
    CerebroTime &add_minute(int n);
    CerebroTime &add_days(int n);

    bool is_trading(CerebroCalendar *cal);
    CerebroTime next_timepoint(CerebroCalendar *cal, int offset = 0);
    CerebroTime next_date(CerebroCalendar *cal, int offset = 0);

  private:
    int _time; // 时间
};

class CerebroDate final
{
  public:
    CerebroDate(int dt);
    CerebroDate(const CerebroDate &ct);
    int64_t ts() const;
    int time() const; // 返回时间对象
    CerebroDate &add_days(int n);

    bool is_trading(CerebroCalendar *cal);
    CerebroDate next_date(CerebroCalendar *cal, int offset = 0);

  private:
    int _time; // 时间
};


struct CerebroSeriesValue
{
    int date;
    double value;
};
struct CerebroSectionValue
{
    Symbol symbol;
    double value;
};

// 区间组合， 用于成份股调入调出， ST或者停复牌
struct CerebroComponent
{
    std::vector<std::pair<int,int>> component; // [start_date, end_date] 闭区间组合
    const std::pair<int,int>* contain(int date) const; // 日期在组合内
};


struct CerebroConfig
{
  std::string name; // 账户名
  int64_t aid = 0; // 账户ID
  int64_t strategyid; // 策略ID，一个策略一个账户ID，（后续可以支持一个策略用不同账户ID来下单）, 一个账户ID可以在多个策略中使用
  double cash = 1000000;// 初始资金， 金回测用
  double comm_rate = 0.0003; //手续费
  int slippage = 1.0; // 滑点

  std::vector<Symbol> symbols; // 关注的标的,  回放行情时使用
  int start_date; // 回测起始日期
  int end_date;// 回测截至日期
  QUOTES_TYPE qtype = QUOTES_TYPE::TICK; // 撮合行情类型
  STRATEGY_RUN_TYPE rtype = STRATEGY_RUN_TYPE::BACKTEST;
};

// 数据推送
// 订阅时间点，按时间点取数据并推送
// 并提供获取数据的接口
class CerebroDataFeed
{
  public:
    CerebroDataFeed();
    ~CerebroDataFeed();
    int get_kline(const Symbol &symbol, int date, CerebroKlineRecord &record);
    int get_tick(const Symbol &symbol, int date, CerebroTickRecord &record);
    // 获取数据序列
    int get_data_series(const Symbol &symbol, int date, int num, const std::string& fields, std::map<std::string, CerebroSeriesValue>& values);
    // 获取数据截面
    int get_data_section(const std::vector<Symbol>& symbols, int date, const std::string& fields, std::map<std::string, CerebroSectionValue>& values);
    // 获取数据表
    int get_data_table(const Symbol& symbols, int date, int num, const std::string& tblname);
    // 获取成份数据
    int get_component(const Symbol &index, std::map<Symbol, CerebroComponent>& component);

    // 判断ST
    bool is_st(const Symbol &symbol, int date) const;
    // 判断停牌
    bool is_stop( const Symbol &symbol, int date) const;

    // 接受时间点事件
    int on_timepoint();
};

class CerebroStrategy
{
  public:
  CerebroStrategy() = default;
    virtual ~CerebroStrategy();
    virtual void on_market_open(CerebroBroker* broker, int dt) = 0;
    virtual void on_pre_market_open(CerebroBroker* broker, int dt) = 0;
    virtual void on_market_close(CerebroBroker* broker, int dt) = 0;
    virtual void on_aft_market_close(CerebroBroker* broker, int dt) = 0;
    // virtual void on_aft_market_settle(CerebroBroker* broker, int dt) = 0; // 触发broker处理结算事务
    virtual void on_tick(CerebroBroker* broker, CerebroTickRecord* tick) = 0;  // 加载tick缓存， 触发用户业务回调on_tick，再触发broker撮合
    virtual void on_kline(CerebroBroker* broker, CerebroKlineRecord* kline) = 0; // 根据时间点加载k线缓存，再处理用户业务回调on_kline
    // virtual void on_market_quote_data() = 0; // 市场行情数据, 所有行情数据tick,minute,daily都先经过此函数再到on_tick,on_kline
    virtual void on_order_update(CerebroBroker* broker,const CerebroOrder* order) = 0;

    const std::string& name() const { return name_;}
    protected:
    std::string name_;
};
typedef std::shared_ptr<CerebroStrategy> CerebroStrategyPtr;

class CerebroEvent
{
  public:
    enum EventCode
    {
        EVENT_NONE = 0,
        PRE_MARKET_OPEN = 1,   // 开盘前
        PRE_MARKET_CLOSE = 2,  // 收盘前
        PRE_MARKET_SETTLE = 2, // 盘前结算
        MARKET_OPEN = 3,       // 开盘
        MARKET_CLOSE = 4,      // 收盘
        AFT_MARKET_SETTLE = 2, // 盘后结算
        MARKET_FINISH = 2,     // 结束
        TIMEPOINT_ARRIVE = 5, // 时间点到达，用于驱动回测流程(预先将时间按回测周期切片，然后一次灌入事件引擎)
        MAREKT_QUOTE_DATA = 7, // 市场行情数据, tick minute daily ...

        ORDER_UPDATE = 7,
        // ORDER_UPDATE = 8,
    };

    CerebroEvent(EventCode code, int type, void *data) : _code(code), _type(type), _user_data(data) {}
    ~CerebroEvent();

    EventCode code() const { return _code; }
    void set_ts(int64_t ts) { _ts = ts; }

    static int build_event_stream(std::vector<CerebroEvent> &stream); // 构造事件流， 仅回测使用
  private:
    EventCode _code{EVENT_NONE}; // 事件代码
    int _type{0};                // 事件类型
    void *_user_data{nullptr};   // 业务数据
    int64_t _ts{0};              // 当前时间戳
};

class CerebroEventStream
{
  public:
    CerebroEventStream();
    ~CerebroEventStream();
    CerebroEvent *pop(); // 获取最新的事件
    // void push(CerebroEvent* ev);
    void push_back(const CerebroEvent &ev);

    void build(int start_dt, int end_dt) // 数据驱动，先构造好数据，然后事件引擎依次取出再处理
    {
        _events.emplace_back(CerebroEvent(CerebroEvent::PRE_MARKET_OPEN, 0, nullptr));
        _events.emplace_back(CerebroEvent(CerebroEvent::PRE_MARKET_SETTLE, 0, nullptr));

        _events.emplace_back(CerebroEvent(CerebroEvent::MARKET_OPEN, 0, nullptr));
        for (int dt = start_dt; dt <= end_dt; ++dt)
        {
            // 调整时间点, 发生时间点到达事件
            // 日线每天发一次
            // 分钟线按交易事件短分片后发送
            // tick
            //_events.emplace_back(CerebroEvent(CerebroEvent::TIMEPOINT_ARRIVE, 0, nullptr));
            // minute
            //_events.emplace_back(CerebroEvent(CerebroEvent::TIMEPOINT_ARRIVE, 0, nullptr));
            // daily
            _events.emplace_back(CerebroEvent(CerebroEvent::TIMEPOINT_ARRIVE, 0, nullptr));
        }
        _events.emplace_back(CerebroEvent(CerebroEvent::MARKET_CLOSE, 0, nullptr));
        _events.emplace_back(CerebroEvent(CerebroEvent::AFT_MARKET_SETTLE, 0, nullptr));
        _events.emplace_back(CerebroEvent(CerebroEvent::MARKET_FINISH, 0, nullptr));
    }

  private:
    size_t _pos{0}; // 当前事件位置
    std::vector<CerebroEvent> _events;
    int64_t _ts; // 记录时间点，如果是分钟k或tick则特殊处理, 只产生(修改体么破I那天——ARRIVE事件)临时事件对象来驱动流程
};

// 接收事件, broker 应该继承并实现
class CerebroEventSink
{
  public:
    virtual void on_market_open() = 0;
    virtual void on_pre_market_settle() = 0; // 触发broker处理结算事务
    virtual void on_pre_market_open() = 0;
    virtual void on_market_close() = 0;
    virtual void on_aft_market_close() = 0;
    virtual void on_aft_market_settle() = 0; // 触发broker处理结算事务
    virtual void on_tick() = 0;  // 加载tick缓存， 触发用户业务回调on_tick，再触发broker撮合
    virtual void on_kline() = 0; // 根据时间点加载k线缓存，再处理用户业务回调on_kline
    virtual void on_market_quote_data() = 0; // 市场行情数据, 所有行情数据tick,minute,daily都先经过此函数再到on_tick,on_kline
};

class CerebroEngine
{
  public:
    CerebroEngine();
    ~CerebroEngine();
    int init();
    int set_broker(CerebroBroker *broker)
     {_broker = broker; return 0;}
    int set_datafeed(CerebroDataFeed *feed)
     {_feed= feed; return 0;}
    int set_stream(CerebroEventStream *stream)
     {_stream = stream; return 0;}

    void pump(CerebroEvent &ev); // 外部导入事件，模拟盘or实盘用
    CerebroEvent *poll()         // 轮询事件，有事件到达时返回
    {
        return _stream->pop();
    }
    CerebroEvent *run_once() // 处理一次事件
    {
        auto ev = _stream->pop();
        if(ev == nullptr)
        {
          return ev;
        }
        switch (ev->code())
        {
        case CerebroEvent::PRE_MARKET_OPEN /* constant-expression */:
            // 盘前准备
            _sink->on_pre_market_open();
            break;
        case CerebroEvent::PRE_MARKET_SETTLE /* constant-expression */:
            // 盘前结算事务
            _sink->on_pre_market_settle();
            break;
        case CerebroEvent::MARKET_OPEN /* constant-expression */:
            // 处理开盘事件，回调用户函数
            /* code */
            _sink->on_market_open();
            break;
        case CerebroEvent::MAREKT_QUOTE_DATA:
            //_sink->on_tick();
            _sink->on_market_quote_data();
        default:
            break;
        }
        return ev;
    }

    void run()
    {
      while(run_once());
    }

  private:
    CerebroEvent *_event{nullptr}; // 记录当前事件
    CerebroDataFeed* _feed{nullptr};
    CerebroEventStream *_stream{nullptr}; // 事件流
    CerebroEventSink *_sink{nullptr};     // 消费事件
    CerebroBroker *_broker{nullptr};      // 订单管理撮合, 持仓管理，帐号管理
};