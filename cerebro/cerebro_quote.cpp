#include "cerebro_quote.h"

CerebroTickRecord *CerebroQuotePlayer::next()
{
    CerebroTickRecord *tick = nullptr;
    // TODO
    // 过滤未关注的标的
    // 按时间戳回放， 切回交易日时，返回nullptr
    if (records_.empty() && pos_ < 0)
    {
        if (qtype_ == QUOTES_TYPE::DAY)
        {
            std::vector<CerebroKlineRecord> records;
            qprovider_->get_daily_kline(current_dt_, records);
            for (auto const &r : records)
            {
                CerebroTickRecord t;
                kline_to_tick(r, t);
                records_.emplace_back(std::move(t));
            }
            pos_ = 0;
        }
        else if (qtype_ == QUOTES_TYPE::TICK)
        {
            // TODO
            // 处理tick
            // 每次取一分钟，然后逐条播放
        }
        else
        {
            // TODO
            // 处理分钟线
            // 每次取一个周期(N分钟)，然后逐条播放
        }
    }
    else
    {
        if (records_.size() > static_cast<size_t>(pos_))
        {
            tick = &(records_.at(pos_));
        }
        pos_++;
    }
    return tick;
}

void CerebroQuotePlayer::tick_to_kline(const CerebroTickRecord &t, CerebroKlineRecord &k)
{
    k.symbol = t.symbol;
    k.ts = t.ts;
    k.dt = t.dt;
    k.open = t.open;
    k.high = t.high;
    k.low = t.low;
    k.close = t.last;
    k.prev_close = t.prev_close;
    k.volume = t.volume;
    k.turnover = t.turnover;

    k.open_interest = t.open_interest;
    k.prev_open_interest = t.prev_open_interest;
    k.prev_settlement = t.prev_settlement;
    k.settlement = t.settlement;
}

void CerebroQuotePlayer::kline_to_tick(const CerebroKlineRecord &k, CerebroTickRecord &t)
{
    t.symbol = k.symbol;
    t.ts = k.ts;
    t.dt = k.dt;
    t.open = k.open;
    t.high = k.high;
    t.low = k.low;
    t.last = k.close;
    t.prev_close = k.prev_close;
    t.volume = k.volume;
    t.turnover = k.turnover;

    t.open_interest = k.open_interest;
    t.prev_open_interest = k.prev_open_interest;
    t.prev_settlement = k.prev_settlement;
    t.settlement = k.settlement;

    t.ask_vols.resize(5, 0.0);
    t.bid_vols.resize(5, 0.0);
    t.asks.resize(5, 0.0);
    t.bids.resize(5, 0.0);

    t.ask_vols[0] = k.volume;
    t.bid_vols[0] = k.volume;
    t.asks[0] = k.close;
    t.bids[0] = k.close;
}