#pragma once
#include "util/common.h"
#include "util/logdef.h"

typedef std::string Symbol;

// 投资组合-策略所有账户的集合
class CerebroPortfolio
{
  public:
    // accounts // 账户字典
    // TODO
    double cash;               // 可用资金
    double daily_pnl;          // 当日盈亏
    double annualized_returns; // 累计年化收益率
    double daily_returns;      // 当前最新一天的日收益

    double frozen_cash;           // 冻结资金
    double market_value;          // 市值
    double pnl;                   // 收益
    double portfolio_value;       // 总权益
    int start_date;               // 策略投资组合的开始日期
    double starting_cash;         // 初始资金
    double static_unit_net_value; // 昨日净值

    double total_returns;    //
    double total_value;      //
    double transaction_cost; // 交易成本（税费）
    double unit_net_value;   // 实时净值, 根据净值计算累积收益
    double units;            // 份额 
};

// 账户-多种持仓和现金的集合
class CerebroAccount
{
  public:
    // int64_t id;
    // int dt;
    // 总保证金
    double margin;
    // 多方向保证金
    double buy_margin;
    // 空方向保证金
    double sell_margin;
    // 可用资金
    double cash;
    // 账户总资金 (总资产=账户总资金+市值)
    double total_cash;
    // 当日盈亏
    double daily_pnl;
    // 总权益
    double equity;
    // 冻结资金
    double frozen_cash;
    // 市值
    double market_value;
    // 昨仓盈亏
    double position_pnl;
    // 账户总权益
    double total_value;
    // 交易盈亏
    double trading_pnl;
    // 总费用
    double transaction_cost;

    // 份额
    double units; // 初次入金时， 值为 total_cash; 后面中途入金 份额 = (units*unit_value + into_cash)/单位价值
    // 单位价值
    double unit_value;  // 初次入金时， 值为1

    // 根据计算当前不超过 trade_amount 的最大可平仓量
    int calc_close_today_amount();
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
    // 
    BONUS_SHARES = 102 //红利股/送股

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
    // long long base_order_id;

    // 订单开平
    // POSITION_EFFECT position_effect;

    // 订单价格
    double price;

    // 订单量
    double quantity;

    // 扩展字段
    std::map<std::string, std::string> extra; // base_oid 母单order_id 部分成交时子单填写母单order_id; 

    // 订单动作/方向
    ORDER_ACTION action;
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
    double closable = 0;
    // 当前持仓的方向
    POSITION_DIRECTION direction;
    // 当前持仓所占的保证金
    double margin = 0;
    // 当前持仓的市值
    double market_value = 0;
    // 当前持仓的标的
    Symbol symbol;
    // 持仓累积盈亏
    double pnl = 0;
    // 当前持仓当日持仓盈亏
    double position_pnl = 0;
    // 当前持仓量
    double quantity = 0;
    // 今仓中的可平仓位
    double today_closable = 0;
    // 当前持仓当日的交易盈亏
    double trading_pnl = 0;

    // 持仓均价
    double avg_price = 0.0;
    // 总手续费
    double commissions = 0.0;

    // 持仓价值 avg_price * quantity
    double totalCommited = 0.0;
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
    int64_t ts;    // 当前快照数据的时间戳
    int dt;        // 交易日

    std::vector<double> ask_vols; // 卖出报盘数量，ask_vols[0]代表盘口卖一档报盘数量
    std::vector<double> asks;     // 卖出报盘价格，asks[0]代表盘口卖一档报盘价
    std::vector<double> bid_vols; // 买入报盘数量，bids_vols[0]代表盘口买一档报盘数量
    std::vector<double> bids;     // 买入报盘价格，bids[0]代表盘口买一档报盘价
    double high;                  // 截止到当前的最高价
    double last;                  // 当前最新价
    double low;                   // 截止到当前的最低价
    double open;                  // 当日开盘价
    double close;                 // 当前最新价
    double prev_close;            // 昨日收盘价
    double total_turnover;        // 截止到当前的成交额
    double volume;                // 截止到当前的成交量

    double open_interest;      // 截止到当前的持仓量（期货专用）
    double prev_open_interest; // 昨日的持仓量（期货专用）
    double settlement;         // 结算价（期货专用）
    double prev_settlement;    // 昨日结算价（期货专用）

    double limit_down; // 跌停价
    double limit_up;   // 涨停价
};

class CerebroKlineRecord
{
  public:
    Symbol symbol;         // 标的
    int64_t ts;            // 当前快照数据的时间戳
    int type;              // k线类型 1m 5m 1d
    double high;           // 截止到当前的最高价
    double close;          // 当前最新价
    double low;            // 截止到当前的最低价
    double open;           // 当日开盘价
    double prev_close;     // 昨日收盘价
    double total_turnover; // 截止到当前的成交额
    double volume;         // 截止到当前的成交

    double open_interest;      // 截止到当前的持仓量（期货专用）
    double prev_open_interest; // 昨日的持仓量（期货专用）
    double prev_settlement;    // 昨日结算价（期货专用）
    double settlement;         // 结算价（期货专用）
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
}

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

        ORDER_UPDATE = 6,
        // ORDER_UPDATE = 6,
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
            _events.emplace_back(CerebroEvent(CerebroEvent::TIMEPOINT_ARRIVE, 0, nullptr));
            // 日线每天发一次
            // 分钟线按交易事件短分片后发送
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
};

class CerebroEngine
{
  public:
    CerebroEngine();
    ~CerebroEngine();
    int init();
    int set_broker(CerebroBroker *broker);
    int set_datafeed(CerebroDataFeed *feed);

    void pump(CerebroEvent &ev); // 外部导入事件，模拟盘or实盘用
    CerebroEvent *poll()         // 轮询事件，有事件到达时返回
    {
        return _stream->pop();
    }
    CerebroEvent *run_once() // 处理一次事件
    {
        auto ev = _stream->pop();
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

        default:
            break;
        }
        return nullptr;
    }

  private:
    CerebroEvent *_event{nullptr}; // 记录当前事件

    CerebroEventStream *_stream{nullptr}; // 事件流
    CerebroEventSink *_sink{nullptr};     // 消费事件
    CerebroBroker *_broker{nullptr};      // 订单管理撮合, 持仓管理，帐号管理
};