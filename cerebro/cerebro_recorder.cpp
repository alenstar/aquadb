#include "cerebro_recorder.h"

void CerebroQuotePlayer::tick_to_kline(const CerebroTickRecord& t, CerebroKlineRecord& k)
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
    k.prev_settlement= t.prev_settlement;
    k.settlement= t.settlement;
}

void CerebroQuotePlayer::kline_to_tick(const CerebroKlineRecord& k, CerebroTickRecord& t)
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
    t.prev_settlement= k.prev_settlement;
    t.settlement= k.settlement;

    t.ask_vols.resize(5, 0.0);
    t.bid_vols.resize(5, 0.0);
    t.asks.resize(5, 0.0);
    t.bids.resize(5, 0.0);

    t.ask_vols[0] = k.volume;
    t.bid_vols[0] = k.volume;
    t.asks[0] = k.close;
    t.bids[0] = k.close;
}