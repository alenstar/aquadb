#include "cerebro_backtest.h"
#include "cerebro_broker.h"
#include "cerebro_recorder.h"

CerebroBacktest::CerebroBacktest() { broker_ = new CerebroBroker(); }
CerebroBacktest::~CerebroBacktest() { delete broker_; }
nlohmann::json CerebroBacktest::to_json() const
{
    nlohmann::json js;
    js["id"]   = conf_.aid;
    js["name"] = conf_.name;
    js["state"] = state_;
    // js["last_order_id"] = last_order_id_;
    if (account_) {
        js["account"] = account_->to_json();
    }
    return js;
}
  // void CerebroBacktest::set_quote_provider(std::shared_ptr<CerebroQuoteProvider> provider) {
  //    qprovider_ = provider;
  //  }


int CerebroBacktest::init(const CerebroConfig &conf)
{
    conf_ = conf;
    // 设置订单更新回调
    broker_->set_order_update_callback([this](const CerebroOrder &o) {
        for (auto &st : strategys_) {
            // FIXME
            // 可根据订单中的扩展字段找到对于的下单策略
            st.second->on_order_update(broker_, &o);
        }
    });
    last_order_id_ = 1000000;
    broker_->set_order_id_generator([this]() -> int64_t { return last_order_id_++; });

    // 此处账户Id和交易Id保持一样
    account_ = std::make_shared<CerebroAccountWrap>(conf.aid, conf.name);
    broker_->init(account_);
    // 入初始资金
    if(state_ == 0) {
        broker_->deposit_cash(conf.cash);
    }

    broker_->set_matcher("SH", new CerebroSSEMatcher(broker_));
    broker_->set_matcher("SZ", new CerebroSZSEMatcher(broker_));
    return 0;
}
int CerebroBacktest::run()
{
    CerebroKlineRecord kline;
    std::unordered_map<Symbol, CerebroTickRecordPtr> quotes;
    CerebroQuotePlayer player;
    player.init(conf_.qtype, nullptr);

    for (int dt = conf_.start_date; dt <= conf_.end_date; ++dt) {
        // TODO
        // 跳过非交易日
        for (auto &st : strategys_) {
            st.second->on_pre_market_open(broker_, dt);
        }
        // TODO
        int rc = player.seek_to(dt);
        if (rc != 0) {
            LOGE("not found quotes: trading_date=%d", dt);
            continue;
        }

        for (auto &st : strategys_) {
            st.second->on_market_open(broker_, dt);
        }

        CerebroTickRecordPtr quote;
        while (quote = player.next()) {
            for (auto &st : strategys_) {
                if (conf_.qtype != QUOTES_TYPE::TICK) {
                    CerebroQuotePlayer::tick_to_kline(*quote, kline);
                    st.second->on_kline(broker_, &kline);
                }
                else {
                    // tick to kline
                    st.second->on_tick(broker_, quote.get());
                }
            }

            // 可考虑多线程执行
            broker_->on_tick(*quote);
        }

        // 收盘
        for (auto &st : strategys_) {
            st.second->on_market_close(broker_, dt);
        }

        // 盘后结算
        std::vector<CerebroKlineRecord> records;
        rc = player.get_daily_kline(dt, records);
        if(rc != 0) {
            LOGE("get daily kline fail, dt=%d",dt);
        } else {
            broker_->settle(records);
        }
        //收盘后
        for (auto &st : strategys_) {
            st.second->on_aft_market_close(broker_, dt);
        }
    }
    return 0;
}
