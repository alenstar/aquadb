#pragma once
#include "cerebro_struct.h"

class CerebroQuoteProvider
{
  public:
    virtual ~CerebroQuoteProvider() {}
    virtual int get_daily_kline(int dt, std::vector<CerebroKlineRecord> &records) = 0;
    virtual int get_daily_kline_by_range(const Symbol &symbol, int start_dt, int end_dt,
                                         std::vector<CerebroKlineRecord> &records) = 0;
    virtual int get_daily_kline_by_num(const Symbol &symbol, int end_dt, int num,
                                       std::vector<CerebroKlineRecord> &records) = 0;
    virtual int get_minute_kline(const Symbol &symbol, int dt, int span, std::vector<CerebroKlineRecord> &records) = 0;
    virtual int get_minute_kline_by_timestop(int64_t timestop, int span, std::vector<CerebroKlineRecord> &records) = 0;
    virtual bool is_trading_day(int dt) = 0;
    virtual int get_dividend(const Symbol &symbol, int dt,CerebroDividend& dividend) = 0;

    // 获取最新tick
    virtual int get_last_tick(int dt, std::vector<CerebroTickRecord> &records) = 0;
    // virtual int next_tick() = 0;
};
typedef std::shared_ptr<CerebroQuoteProvider> CerebroQuoteProviderPtr;

// 记录行情
// 记录时 key 格式： trading_date + timestamp + symbol (日线不用timestamp字段)
// 此格式用于回放时按事件将标的和行情对齐
class CerebroQuoteRecoder
{
  public:
    CerebroQuoteRecoder() {}
    ~CerebroQuoteRecoder() {}
    int init() { return -1; }

  private:
};

// 播放行情
// 按交易日定位回放的数据块，回放完一个交易日后，调用者需重新定位交易日
// 数据的key格式保证了行情回放顺序
class CerebroQuotePlayer
{
  public:
    CerebroQuotePlayer() {}
    ~CerebroQuotePlayer() {}
    // 按行情类型初始化
    int init(QUOTES_TYPE qtype, CerebroQuoteProvider *qprovider)
    {
        qtype_ = qtype;
        qprovider_ = qprovider;
        return 0;
    }

    bool isvalid() const { return false; }

    // 定位到指定交易日
    int seek_to(int dt)
    {
        current_dt_ = dt;
        pos_ = -1;
        records_.clear();
        return 0;
    }
    CerebroTickRecord *next();

    //int get_daily_kline(int dt, std::vector<CerebroKlineRecord> &records)
    //{
    //    // TODO
    //    return -1;
    //}
    static void tick_to_kline(const CerebroTickRecord &t, CerebroKlineRecord &k);
    static void kline_to_tick(const CerebroKlineRecord &k, CerebroTickRecord &t);

  protected:
    int seek_tick_to(int dt);
    int seek_minute_to(int dt);
    int seek_day_to(int dt);

  private:
    // record pos
    QUOTES_TYPE qtype_{QUOTES_TYPE::DAY};
    CerebroQuoteProvider *qprovider_{nullptr};
    int current_dt_{0};
    std::vector<CerebroTickRecord> records_;
    int64_t pos_{-1};
};
