#include "cerebro_backtest.h"
#include "cerebro_broker.h"
#include "cerebro_quote.h"
#include "util/datetime.h"

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
        // 策略回调
        for (auto &st : strategys_) {
            // FIXME
            // 可根据订单中的扩展字段找到对于的下单策略
            st.second->on_order_update(broker_, &o);
        }
        if(o.status != ORDER_STATUS::FILLED)
        {
            return;
        }
        // 分析器回调
        for (auto &an : analyzers_) {
            // FIXME
            // 实时计算策略指标
            an.second->on_order_update(broker_, &o);
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
int CerebroBacktest::run(CerebroQuoteProvider* provider)
{
    CerebroKlineRecord kline;
    std::unordered_map<Symbol, CerebroTickRecordPtr> quotes;
    CerebroQuotePlayer player;
    player.init(conf_.qtype, provider);
    util::DateTime start_dt(static_cast<util::DateTime::DateType>(conf_.start_date));
    util::DateTime end_dt(static_cast<util::DateTime::DateType>(conf_.end_date));

    for (auto dt = start_dt; dt <= end_dt; dt.add_days(1)) {
        // TODO
        // 跳过非交易日
        if(dt.is_weekend())
        {
            continue;
        }
        // TODO
        int rc = player.seek_to(static_cast<int>(dt.date()));
        if (rc != 0) {
            LOGE("not found quotes: trading_date=%d", dt.date());
            continue;
        }

        for (auto &st : strategys_) {
            st.second->on_pre_market_open(broker_, static_cast<int>(dt.date()));
        }

        for (auto &st : strategys_) {
            st.second->on_market_open(broker_, static_cast<int>(dt.date()));
        }

        CerebroTickRecord* quote = nullptr;
        while (quote = player.next()) {
            // 策略回调
            for (auto &st : strategys_) {
                if (conf_.qtype != QUOTES_TYPE::TICK) {
                    CerebroQuotePlayer::tick_to_kline(*quote, kline);
                    st.second->on_kline(broker_, &kline);
                }
                else {
                    // tick to kline
                    st.second->on_tick(broker_, quote);
                }
            }

            // broker 回调，处理订单
            // 可考虑多线程执行
            broker_->on_tick(*quote);

            // 分析器回调
            // 更新收益等
            for(auto& an: analyzers_)
            {
                an.second->on_tick(broker_, quote);
            }
        }

        // 收盘
        for (auto &st : strategys_) {
            st.second->on_market_close(broker_,  static_cast<int>(dt.date()));
        }

        // 盘后结算
        std::vector<CerebroKlineRecord> records;
        rc = player.get_daily_kline( static_cast<int>(dt.date()), records);
        if(rc != 0) {
            LOGE("get daily kline fail, dt=%d",  static_cast<int>(dt.date()));
        } else {
            broker_->settle(records);
        }

            // 分析器回调
            // 更新收益等
            for(auto& an: analyzers_)
            {
                an.second->on_settle(broker_);
            }

        //收盘后
        //for (auto &st : strategys_) {
        //    st.second->on_aft_market_close(broker_,  static_cast<int>(dt.date()));
        //}
    }
    return 0;
}
