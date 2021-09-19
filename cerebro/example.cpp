#include "cerebro_backtest.h"
#include "cerebro_broker.h"
#include "cerebro_struct.h"
#include "cerebro_trader.h"
#include "sina_api.h"
#include "tencent_api.h"
#include "util/datetime.h"

#include <arrow/array.h>
#include <arrow/chunked_array.h>
#include <arrow/io/api.h>
#include <arrow/ipc/feather.h>
#include <arrow/table.h>
#include <gflags/gflags.h>

#include <chrono>
#include <iostream>
INITIALIZE_EASYLOGGINGPP

DEFINE_int32(start_date, 20200101, "backtest start date");
DEFINE_int32(end_date, 20210901, "backtest end date");
DEFINE_string(filepath, "daily-000001.SZ.parquet", "kline parquet file path");

class MyQuoteProvider : public CerebroQuoteProvider
{
  public:
    MyQuoteProvider() {}
    ~MyQuoteProvider() override {}
    int get_daily_kline(int dt, std::vector<CerebroKlineRecord> &records) override
    {
        // TODO
        auto it = dt2pos_.find(dt);
        if (it == dt2pos_.cend())
        {
            LOGE("not found trade_date:%d", dt);
            return -1;
        }

        auto code = std::static_pointer_cast<arrow::StringArray>(table_->GetColumnByName("code")->chunk(0));
        auto trade_date = std::static_pointer_cast<arrow::Date32Array>(table_->GetColumnByName("trade_date")->chunk(0));
        auto open = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("open")->chunk(0));
        auto high = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("high")->chunk(0));
        auto low = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("low")->chunk(0));
        auto close = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("close")->chunk(0));
        auto preclose = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("pre_close")->chunk(0));
        auto vol = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("vol")->chunk(0));
        auto amount = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("amount")->chunk(0));

        //int64_t rows_num = table_->num_rows();
        for (int64_t i = it->second.first; i <= it->second.second; ++i)
        {
            CerebroKlineRecord kline;
            kline.symbol = code->Value(i).to_string();
            kline.dt = dt;
            kline.open = open->Value(i);
            kline.high = high->Value(i);
            kline.low = low->Value(i);
            kline.close = close->Value(i);
            kline.preclose = preclose->Value(i);
            kline.volume = vol->Value(i);
            kline.turnover = amount->Value(i);
            records.emplace_back(std::move(kline));
        }
        return 0;
    }
    int get_last_tick(int dt, std::vector<CerebroTickRecord> &records) override
    {
        // TODO
        auto it = dt2pos_.find(dt);
        if (it == dt2pos_.cend())
        {
            LOGE("not found trade_date:%d", dt);
            return -1;
        }

        auto code = std::static_pointer_cast<arrow::StringArray>(table_->GetColumnByName("code")->chunk(0));
        auto trade_date = std::static_pointer_cast<arrow::Date32Array>(table_->GetColumnByName("trade_date")->chunk(0));
        auto open = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("open")->chunk(0));
        auto high = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("high")->chunk(0));
        auto low = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("low")->chunk(0));
        auto close = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("close")->chunk(0));
        auto preclose = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("pre_close")->chunk(0));
        auto vol = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("vol")->chunk(0));
        auto amount = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("amount")->chunk(0));

        //int64_t rows_num = table_->num_rows();
        for (int64_t i = it->second.first; i <= it->second.second; ++i)
        {
            CerebroTickRecord t;
            t.symbol = code->Value(i).to_string();
            t.dt = dt;
            t.ts = util::DateTime(static_cast<util::DateTime::DateType>(dt), 15,0,0).time() * 1000LL;
            t.open = open->Value(i);
            t.high = high->Value(i);
            t.low = low->Value(i);
            t.last = close->Value(i);
            t.preclose = preclose->Value(i);
            t.volume = vol->Value(i);
            t.turnover = amount->Value(i);

                t.limit_down = t.preclose * 0.9; //模拟跌停价
    t.limit_up = t.preclose * 1.1; //模拟涨停价

    t.ask_vols.resize(5, 0.0);
    t.bid_vols.resize(5, 0.0);
    t.asks.resize(5, 0.0);
    t.bids.resize(5, 0.0);

    t.ask_vols[0] = t.volume;
    t.bid_vols[0] = t.volume;
    t.asks[0] = t.last;
    t.bids[0] = t.last;

            records.emplace_back(std::move(t));
        }
        return 0;
    }

    int get_daily_kline_by_range(const Symbol &symbol, int start_dt, int end_dt,
                                 std::vector<CerebroKlineRecord> &records) override
    {
        // TODO
        return -1;
    }
    int get_daily_kline_by_num(const Symbol &symbol, int end_dt, int num,
                               std::vector<CerebroKlineRecord> &records) override
    {
        // TODO
        return -1;
    }
    int get_minute_kline(const Symbol &symbol, int dt, int span, std::vector<CerebroKlineRecord> &records) override
    {
        // TODO
        return -1;
    }
    int get_minute_kline_by_timestop(int64_t timestop, int span, std::vector<CerebroKlineRecord> &records) override
    {
        // TODO
        return -1;
    }

    // 获取分红数据
    int get_dividend(const Symbol &symbol, int dt,CerebroDividend& dividend) override
    {
        auto it = dividends_.find(dt);
        if(it == dividends_.cend())
        {
            return -1;
        }
        dividend = it->second;
        return 0;
    }

    bool is_trading_day(int dt) override
    {
        auto it = dt2pos_.find(dt);
        if (it == dt2pos_.cend())
        {
            // LOGE("not found trade_date:%d", dt);
            return false;
        }
        return true;
    }
    int load_feather(const std::string &filepath)
    {
        // TODO
        arrow::Status st;
        arrow::MemoryPool *pool = arrow::default_memory_pool();
        std::shared_ptr<arrow::io::ReadableFile> infile =
            arrow::io::ReadableFile::Open(filepath, pool).MoveValueUnsafe();
        auto reader = arrow::ipc::feather::Reader::Open(infile).MoveValueUnsafe();
        std::shared_ptr<arrow::Table> table = nullptr;
        st = reader->Read(&table);
        // Read entire file as a single Arrow table
        if (!st.ok())
        {
            LOGE("read table failed. [%d]%s", static_cast<int>(st.code()), st.message().c_str());
            return -1;
        }
        table_ = table->CombineChunks(pool).MoveValueUnsafe();
        // for DEBUG
        if (1)
        {
            auto desc = table_->schema()->ToString();
            std::cout << "file:" << filepath << " schema:" << std::endl;
            std::cout << desc << std::endl;
        }
        return 0;
    }

    int preload(int start_dt, int end_dt)
    {
        auto trade_date = std::static_pointer_cast<arrow::Date32Array>(table_->GetColumnByName("trade_date")->chunk(0));
        int64_t rows_num = table_->num_rows();
        for (int64_t i = 0; i < rows_num; ++i)
        {
            int dt = util::tm2dateint(static_cast<time_t>(trade_date->Value(i) * (3600 * 24)));
            // LOGD("dt=%d, date32:%d", dt, trade_date->Value(i));
            if (dt > end_dt || dt < start_dt)
            {
                continue;
            }
            // LOGD("find trade_date:%d", dt);
            auto it = dt2pos_.find(dt);
            if (it == dt2pos_.cend())
            {
                // LOGD("first find trade_date:%d", dt);
                dt2pos_[dt] = std::make_pair(i, i);
            }
            else
            {
                if (i > it->second.second)
                {
                    it->second.second = i;
                }
                else if (i < it->second.first)
                {
                    it->second.first = i;
                }
            }
        }

        // 加载分红信息
        // 通过k线的close和preclose简单计算出分红信息
        auto closes = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("close")->chunk(0));
        auto precloses = std::static_pointer_cast<arrow::DoubleArray>(table_->GetColumnByName("pre_close")->chunk(0));
        auto symbols = std::static_pointer_cast<arrow::StringArray>(table_->GetColumnByName("code")->chunk(0));
        for (int64_t i = 1; i < rows_num; ++i)
        {
            int dt = util::tm2dateint(static_cast<time_t>(trade_date->Value(i) * (3600 * 24)));
            int predt = util::tm2dateint(static_cast<time_t>(trade_date->Value(i - 1) * (3600 * 24)));
            double close =  closes->Value(i - 1);
            double preclose =  precloses->Value(i);
            // 获取标的代码，按标的存储
            // TODO

            if(util::almost_eq(close, preclose, 0.00001) || std::isnan(preclose))
            {
                continue;
            }
            CerebroDividend dividend;
            dividend.symbol = symbols->Value(i).to_string();
            dividend.dividend_date = dt;
            dividend.closure_date  = predt;
            dividend.pub_date = predt; // 假设的
            dividend.round_lot = 10; // 分红最小单位 A股当作是10
            // 价格变化当作现金分红
            dividend.dividend_cash = (close - preclose) * 10;

            dividends_[dividend.dividend_date] = std::move(dividend);
            LOGD("dividend: %s %d %d cash=%f share=%f", dividend.symbol.c_str(), dividend.dividend_date, dividend.closure_date, dividend.dividend_cash, dividend.dividend_share);
        }
        return 0;
    }

  private:
    std::unordered_map<Symbol, std::vector<CerebroKlineRecord>> records_;
    std::shared_ptr<arrow::Table> table_{nullptr};
    std::map<int, std::pair<int64_t, int64_t>> dt2pos_; // 快速定位用

    // 单个标的，多个标的是要再加一层map
    std::map<int, CerebroDividend> dividends_; // 分红信息
};

class MyAnalyzer : public CerebroAnalyzer
{
  public:
    MyAnalyzer(const CerebroConfig* conf):conf_(conf) { name_ = "MyAnalyzer"; }
    ~MyAnalyzer() override {}
    void on_tick(CerebroBroker *broker, CerebroTickRecord *tick) override
    {
        // 计算实时收益
    }
    void on_order_update(CerebroBroker *broker, const CerebroOrder *order) override
    {
        // 计算平仓胜率
        if(order->status != ORDER_STATUS::FILLED)
        {
            return;
        }

        // 卖出才视为完整的交易
        if(order->action == ORDER_ACTION::SELL || order->action == ORDER_ACTION::BUY_TO_COVER)
        {
            //LOGD("sell: %s %f, %f %f", order->symbol.c_str(), order->filled_quantity, order->filled_price, order->last_position_price);
            total_trading_count_++;
            if(order->filled_price > order->last_position_price)
            {
            LOGD("win sell: %s %f, %f %f", order->symbol.c_str(), order->filled_quantity, order->filled_price, order->last_position_price);
                win_trading_count_++;
            }
        }
    }
    void on_settle(CerebroBroker *broker)
    {
        net_values_.emplace_back(std::make_pair(broker->now_date(),broker->current_account()->unit_value()));
        // 计算结算后的收益率
        trading_days_++;

        // 年化收益
        // 回测简化计算直接用初始入金作为本金(回测中途不入金)
        // 每年252个交易日 
        annualized_return_ = 100.0 * (( broker->current_account()->market_value() + broker->current_account()->total_cash() - conf_->cash) /conf_->cash)/(trading_days_/252.0);
        accumulated_return_ = 100 * (broker->current_account()->unit_value() - 1)/1;
        maximum_drawdown_ = std::max(maximum_drawdown_, __maximum_drawdown());
        account_ = broker->current_account()->account();
        LOGD("date:%d market_value:%f, total_cash:%f units:%f unit_value:%f %f", broker->now_date(),  
                    broker->current_account()->market_value(), broker->current_account()->total_cash(),
                     broker->current_account()->units(), broker->current_account()->unit_value(), maximum_drawdown_); 
    }
    double annualized_return() const { return annualized_return_;}
    double win_rate() const {
        if(total_trading_count_ == 0){ return 0;}
        return (win_trading_count_/static_cast<double>(total_trading_count_)) * 100;
    }
    double __maximum_drawdown()const  {
        auto maxit = std::max_element(net_values_.cbegin(), net_values_.cend(), [](decltype(*net_values_.cbegin()) a, decltype(*net_values_.cbegin()) b){ return a.second < b.second;});
        if(maxit == net_values_.cend())
        {
            return 0;
        }
        auto minit = std::min_element(maxit, net_values_.cend(), [](decltype(*net_values_.cbegin()) a, decltype(*net_values_.cbegin()) b){ return a.second < b.second;});
        if(minit == net_values_.cend())
        {
            return 0;
        }

        return 100 * (maxit->second - minit->second)/maxit->second;
    };
    double maximum_drawdown()const  { return maximum_drawdown_;};
    double accumulated_return()const {return accumulated_return_;}
    const CerebroAccount& account() const {return account_;}
    size_t trading_days() const {return trading_days_;}
    size_t trading_count() const {return total_trading_count_;}
private:
    const CerebroConfig* conf_{nullptr};
    double annualized_return_{0};
    double accumulated_return_{0};
    double maximum_drawdown_{0};
    //double win_rate{0};
    size_t total_trading_count_{0};
    size_t win_trading_count_{0};
    size_t trading_days_{0};
    double max_net_value_{0};
    //std::map<int, double> net_values_;
    CerebroAccount account_;
    std::vector<std::pair<int, double>> net_values_;
};

class MyStrategy : public CerebroStrategy
{
  public:
    MyStrategy() { name_ = "MyStrategy"; }
    ~MyStrategy() override {}
    void on_market_open(CerebroBroker *broker, int dt) override { LOGD("dt=%d", dt); }
    void on_pre_market_open(CerebroBroker *broker, int dt) override { LOGD("dt=%d", dt); }
    void on_market_close(CerebroBroker *broker, int dt) override { LOGD("dt=%d", dt); }
    void on_aft_market_close(CerebroBroker *broker, int dt) override { LOGD("dt=%d", dt); }
    // 加载tick缓存， 触发用户业务回调on_tick，再触发broker撮合
    void on_tick(CerebroBroker *broker, CerebroTickRecord *tick) override { LOGD("symbol=%s", tick->symbol.c_str()); }
    // 根据时间点加载k线缓存，再处理用户业务回调on_kline
    void on_kline(CerebroBroker *broker, CerebroKlineRecord *kline) override
    {
        LOGD("%d symbol=%s", kline->dt, kline->symbol.c_str());
        auto p = broker->get_long_tracker(kline->symbol);
        if(p == nullptr)
        {
            // buy
            broker->buy(kline->symbol, 1000);
        LOGD("%d buy symbol=%s", kline->dt, kline->symbol.c_str());
        }
        else 
        {
            if(p->empty()) {
                // buy
            broker->buy(kline->symbol, 1000);
        LOGD("%d buy symbol=%s", kline->dt, kline->symbol.c_str());
            }
            else {
            // sell
            broker->sell(kline->symbol, 1000);
        LOGD("%d sell symbol=%s", kline->dt, kline->symbol.c_str());
            }
        }
    }
    void on_order_update(CerebroBroker *broker, const CerebroOrder *order) override
    {
        LOGD("symbol=%s,%f,%f, %d", order->symbol.c_str(), order->price, order->quantity,
             static_cast<int>(order->status));
    }
};

int main(int argc, char *argv[])
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    int rc = 0;

    // 初始化回测模块
    // backtesting
    CerebroConfig config;
    config.cash = 1000000;
    config.qtype = QUOTES_TYPE::DAY;
    config.rtype = STRATEGY_RUN_TYPE::BACKTEST;
    config.comm_rate = 0.0003;
    config.slippage = 1;
    config.start_date = FLAGS_start_date;
    ;
    config.end_date = FLAGS_end_date;
    CerebroBacktest bt;
    rc = bt.init(config);
    if (rc != 0)
    {
        LOGE("backtest init fail, rc=%d", rc);
        return 1;
    }

    // 加载行情数据
    MyQuoteProvider myquote;
    // myquote.preload(config.start_date, config.end_date);
    myquote.load_feather(FLAGS_filepath);
    myquote.preload(config.start_date, config.end_date);

    // 添加策略
    MyStrategy strategy;
    bt.add_strategy(&strategy);

    // 添加分析器
    MyAnalyzer analyzer(&config);
    bt.add_analyzer(&analyzer);

    // 运行回测
    rc = bt.run(&myquote);
    if (rc != 0)
    {
        LOGE("backtest reutrn_code: %d", rc);
        return 2;
    }
    // std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "trading range:" <<  config.start_date << "~" << config.end_date << std::endl;
    std::cout << "trading days:" <<  analyzer.trading_days() << std::endl;
    std::cout << "trading count:" <<  analyzer.trading_count() << std::endl;
    std::cout << "win rate:" <<  analyzer.win_rate() << std::endl;
    std::cout << "maximum drawdown:" <<  analyzer.maximum_drawdown() << std::endl;
    std::cout << "annualized return:" <<  analyzer.annualized_return() << std::endl;
    std::cout << "accumulated return:" <<  analyzer.accumulated_return() << std::endl;
    std::cout << "cash:" <<  analyzer.account().total_cash() << std::endl;
    std::cout << "market value:" <<  analyzer.account().market_value << std::endl;
    std::cout << "transaction_cost:" <<  analyzer.account().transaction_cost << std::endl;
    return 0;
}
