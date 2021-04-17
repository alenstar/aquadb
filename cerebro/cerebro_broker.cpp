#include "cerebro_broker.h"


inline POSITION_DIRECTION order_longshort(const CerebroOrder &order)
{
    if (order.action == ORDER_ACTION::BUY || order.action == ORDER_ACTION::SELL)
    {
        return POSITION_DIRECTION::LONG;
    }
    else if (order.action == ORDER_ACTION::SELL_SHORT || order.action == ORDER_ACTION::BUY_TO_COVER)
    {
        return POSITION_DIRECTION::SHORT;
    }
    return POSITION_DIRECTION::NONE;
}

inline bool is_special_order(const CerebroOrder &order)
{
    switch (order.action)
    {
    case ORDER_ACTION::CANCEL:
    case ORDER_ACTION::AMEND:
    case ORDER_ACTION::BONUS_SHARES:
    case ORDER_ACTION::DIVIDEND:
        return true;
    default:
        break;
    }
    return false;
}

void CerebroPositionTracker::buy(double quantity, double price, double commission)
{
    // TODO
    if (util::almost_eq(_lposition.quantity, 0.0))
    {
        // new position
        _lposition.avg_price = price;
        _lposition.quantity = quantity;
        _lposition.commissions = commission;
    }
    else
    {
        _lposition.avg_price =
            (_lposition.avg_price * _lposition.quantity + quantity * price) / (_lposition.quantity + quantity);
        _lposition.quantity += quantity;
        _lposition.commissions += commission;
    }
}
void CerebroPositionTracker::sell(double quantity, double price, double commission)
{
    // TODO
    if (util::almost_eq(_lposition.quantity, 0.0))
    {
        // 没有持仓, 反向开空？
        // new position
        _lposition.avg_price = price;
        _lposition.quantity = quantity * (-1);
        _lposition.commissions = commission;
    }
    else
    {
        _lposition.avg_price =
            (_lposition.avg_price * _lposition.quantity - quantity * price) / (_lposition.quantity - quantity);
        _lposition.quantity -= quantity;
        _lposition.commissions += commission;
    }
}

void CerebroPositionTracker::buy_to_cover(double quantity, double price, double commission)
{
    // TODO
    // 买平
    if (util::almost_eq(_sposition.quantity, 0.0))
    {
        // new position
        _sposition.avg_price = price;
        _sposition.quantity = quantity;
        _sposition.commissions = commission;
    }
    else
    {
        _sposition.avg_price =
            (_sposition.avg_price * _sposition.quantity + quantity * price) / (_sposition.quantity + quantity);
        _sposition.quantity += quantity;
        _sposition.commissions += commission;
    }
}
void CerebroPositionTracker::sell_short(double quantity, double price, double commission)
{
    // TODO
    //  买空
    if (util::almost_eq(_sposition.quantity, 0.0))
    {
        // new position
        _sposition.avg_price = price;
        _sposition.quantity = quantity;
        _sposition.commissions = commission;
    }
    else
    {
        _sposition.avg_price =
            (_sposition.avg_price * _sposition.quantity + quantity * price) / (_sposition.quantity + quantity);
        _sposition.quantity += quantity;
        _sposition.commissions += commission;
    }
}

// 撮合器， 一个交易所一个撮合器， 内部再按品种分不同处理模块
CerebroMatcher::CerebroMatcher(CerebroBroker *broker) : _broker(broker) {}
CerebroMatcher::~CerebroMatcher() {}

// int CerebroMatcher::do_order_update(const CerebroOrder &order)
//{
//    // TODO
//    // _broker->on_order_update(order);
//    return 0;
//}

int CerebroMatcher::process_limit_order(CerebroOrder &order, const CerebroTickRecord &tick)
{
    if (order.action == ORDER_ACTION::BUY)
    {
        if (tick.bids.size())
        {
            // TODO
            if (!std::isnormal(tick.asks.at(0)))
            {
                // 无效行情
                LOGW("invlaid tick.last=%f", tick.last);
                return 0;
            }
            if (util::almost_nq(order.price, tick.asks.at(0)))
            {
                LOGW("price not match tick.asks0=%f order.price=%f", tick.asks.at(0), order.price);
                return 0;
            }
            order.filled_price = tick.asks.at(0) + _broker->get_slippage();
            order.filled_quantity = std::min(tick.ask_vols.at(0), order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);

            order.status = ORDER_STATUS::FILLED;
            if (order.quantity > tick.ask_vols.at(0))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                // 子单在broker中拆分
                // 处理部分成交
                // 母单extr字段中记录子单id
                // TODO
                // 等待下个行情
            }
            return 0;
        }
        else
        {
            if (!std::isnormal(tick.last))
            {
                // 无效行情
                LOGW("invlaid tick.last=%f", tick.last);
                return 0;
            }
            if (util::almost_nq(order.price, tick.last))
            {
                LOGW("price not match tick.last=%f order.price=%f", tick.last, order.price);
                return 0;
            }
            order.filled_price = tick.last + _broker->get_slippage();
            order.filled_quantity = std::min(tick.volume, order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);
            order.status = ORDER_STATUS::FILLED;
            if (order.quantity > tick.volume)
            {
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                return 0;
            }
        }
    }
    else if (order.action == ORDER_ACTION::SELL)
    {
        if (tick.bids.size())
        {
            // TODO
            if (!std::isnormal(tick.bids.at(0)))
            {
                // 无效行情
                LOGW("invlaid tick.bids0=%f", tick.bids.at(0));
                return 0;
            }
            if (util::almost_nq(order.price, tick.bids.at(0)))
            {
                LOGW("price not match tick.bids0=%f order.price=%f", tick.bids.at(0), order.price);
                return 0;
            }
            order.filled_price = tick.bids.at(0) + _broker->get_slippage();
            order.filled_quantity = std::min(tick.bid_vols.at(0), order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);

            order.status = ORDER_STATUS::FILLED;
            if (order.quantity > tick.bid_vols.at(0))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                // 子单在broker中拆分
                // 处理部分成交
                // 母单extr字段中记录子单id
                // TODO
                // 等待下个行情
            }
            return 0;
        }
        else
        {
            if (!std::isnormal(tick.last))
            {
                // 无效行情
                LOGW("invlaid tick.last=%f", tick.last);
                return 0;
            }
            if (util::almost_nq(order.price, tick.last))
            {
                LOGW("price not match tick.last=%f order.price=%f", tick.last, order.price);
                return 0;
            }
            order.filled_price = tick.last + _broker->get_slippage();
            order.filled_quantity = std::min(tick.volume, order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);
            order.status = ORDER_STATUS::FILLED;
            if (order.quantity > tick.volume)
            {
                // 处理部分成交
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                return 0;
            }
        }
    }
    else
    {
        // TODO
        LOGE("unknown order.action=%d", order.action);
    }
    return 0;
}
int CerebroMatcher::process_market_order(CerebroOrder &order, const CerebroTickRecord &tick)
{
    if (order.action == ORDER_ACTION::BUY)
    {
        if (tick.bids.size())
        {
            // TODO
            if (!std::isnormal(tick.asks.at(0)))
            {
                // 无效行情
                LOGW("invlaid tick.last=%f", tick.last);
                return 0;
            }
            order.filled_price = tick.asks.at(0) + _broker->get_slippage();
            order.filled_quantity = std::min(tick.ask_vols.at(0), order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);
            order.status = ORDER_STATUS::FILLED;
            if (order.quantity > tick.ask_vols.at(0))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
            }
            return 0;
        }
        else
        {
            if (!std::isnormal(tick.last))
            {
                // 无效行情
                LOGW("invlaid tick.last=%f", tick.last);
                return 0;
            }
            order.filled_price = tick.last + _broker->get_slippage();
            order.filled_quantity = std::min(tick.volume, order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);
            order.status = ORDER_STATUS::FILLED;
            if (order.quantity > tick.volume)
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                return 0;
            }
        }
    }
    else if (order.action == ORDER_ACTION::SELL)
    {
        if (tick.bids.size())
        {
            // TODO
            if (!std::isnormal(tick.bids.at(0)))
            {
                // 无效行情
                LOGW("invlaid tick.last=%f", tick.last);
                return 0;
            }
            order.filled_price = tick.bids.at(0) + _broker->get_slippage();
            order.filled_quantity = std::min(tick.bid_vols.at(0), order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);
            order.status = ORDER_STATUS::FILLED;

            if (order.quantity > tick.bid_vols.at(0))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
            }
            return 0;
        }
        else
        {
            if (!std::isnormal(tick.last))
            {
                // 无效行情
                LOGW("invlaid tick.last=%f", tick.last);
                return 0;
            }
            order.filled_price = tick.last + _broker->get_slippage();
            order.filled_quantity = std::min(tick.volume, order.quantity);
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            order.transaction_cost = _broker->calc_commission(order);
            order.status = ORDER_STATUS::FILLED;
            if (order.quantity > tick.volume)
            {
                // 处理部分成交
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                return 0;
            }
        }
    }
    else
    {
        // TODO
        LOGE("unknown order.action=%d", order.action);
    }
    return 0;
}
int CerebroMatcher::process_order_amend(CerebroOrder &order, const CerebroTickRecord &tick)
{
    // TODO
    return 0;
}
int CerebroMatcher::process_order_cancel(CerebroOrder &order, const CerebroTickRecord &tick)
{
    // TODO
    return -1;
}

CerebroSSEMatcher::CerebroSSEMatcher(CerebroBroker *broker) : CerebroMatcher(broker) {}
CerebroSSEMatcher::~CerebroSSEMatcher() {}

int CerebroSSEMatcher::match(CerebroOrder &order, const CerebroTickRecord &tick)
{
    // TODO
    // 根据行情撮合
    // tick.last == order.price
    // 市价单
    if (order.type == ORDER_TYPE::MARKET)
    {
        return process_market_order(order, tick);
    }
    // 限价单
    else if (order.type == ORDER_TYPE::LIMIT)
    {
        return process_limit_order(order, tick);
    }
    else
    {
        // TODO
        LOGE("unknown order.type=%d", order.type);
    }

    // 无效订单，标记，并更新订单
    // 成交,更新订单, broker来更新
    // do_order_update(order);
    return -1;
}

CerebroSZSEMatcher::CerebroSZSEMatcher(CerebroBroker *broker) : CerebroMatcher(broker) {}
CerebroSZSEMatcher::~CerebroSZSEMatcher() {}
int CerebroSZSEMatcher::match(CerebroOrder &order, const CerebroTickRecord &tick)
{
    // TODO
    // 根据行情撮合
    // tick.last == order.price

    // 无效订单，标记，并更新订单
    // 成交,更新订单
    // do_order_update(order);
    return -1;
}

void CerebroBroker::process_special_order(const CerebroOrder &order)
{
    // 处理特殊订单， 例如： 分红送红股，入金等
    // TODO
    switch (order.action)
    {
    case ORDER_ACTION::CANCEL:
    // TODO
    break;
    case ORDER_ACTION::AMEND:
    // TODO
    break;
    case ORDER_ACTION::BONUS_SHARES:
    // TODO
    break;
    case ORDER_ACTION::DIVIDEND:
    // TODO
    break;
    default:
    // ERROR
        break;
    }
}
void CerebroBroker::add_order(const CerebroOrder &order) { _orders.push_back(order); }

void CerebroBroker::on_order_update(const CerebroOrder &order)
{
    if (is_special_order(order))
    {
        // TODO
        // 处理特殊订单， 例如： 分红送红股，入金等
        return process_special_order(order);
    }

    // TODO
    if (order.status != ORDER_STATUS::FILLED)
    {
        // 未成交订单，只做持久化
        return;
    }

    // 更新持仓
    if (order_longshort(order) == POSITION_DIRECTION::LONG)
    {
        auto tracker = get_long_tracker(order.symbol);
        // 多仓方向
        double price = order.filled_price;
        double quantity = order.filled_quantity;
        double commission = order.transaction_cost;

        if (order.action == ORDER_ACTION::BUY)
        {
            tracker->buy(order.filled_quantity, order.filled_quantity, order.transaction_cost);
        }
        else if (order.action == ORDER_ACTION::SELL)
        {
            tracker->sell(order.filled_quantity, order.filled_quantity, order.transaction_cost);
        }
        else
        {
            // TODO
        }
    }
    else if (order_longshort(order) == POSITION_DIRECTION::SHORT)
    {
        auto tracker = get_long_tracker(order.symbol);
        // 空仓方向
        if (order.action == ORDER_ACTION::BUY)
        {
            tracker->buy_to_cover(order.filled_quantity, order.filled_quantity, order.transaction_cost);
        }
        else if (order.action == ORDER_ACTION::SELL)
        {
            tracker->sell_short(order.filled_quantity, order.filled_quantity, order.transaction_cost);
        }
        else
        {
            // TODO
            // handle error
        }
    }
    else
    {
        // 异常订单
        return;
    }
}

// 交易接口，必须返回订单号
int64_t CerebroBroker::buy(const Symbol &symbol, double quantity, double price)
{
    CerebroOrder order;
    order.symbol = symbol;
    order.ctime = 0;
    order.price = price;
    order.quantity = quantity;
    order.status = ORDER_STATUS::PENDING_NEW;
    order.type = price == 0.0 ? ORDER_TYPE::MARKET : ORDER_TYPE::LIMIT;
    order.action = ORDER_ACTION::BUY;
    // TODO
    add_order(order);
    return 0;
    ;
}
int64_t CerebroBroker::sell(const Symbol &symbol, double quantity, double price)
{
    CerebroOrder order;
    order.symbol = symbol;
    order.ctime = 0;
    order.price = price;
    order.quantity = quantity;
    order.status = ORDER_STATUS::PENDING_NEW;
    order.type = price == 0.0 ? ORDER_TYPE::MARKET : ORDER_TYPE::LIMIT;
    order.action = ORDER_ACTION::SELL;
    // TODO
    add_order(order);
    return 0;
}

int64_t CerebroBroker::buy_to_cover(const Symbol &symbol, double quantity, double price)
{
    // TODO
    return 0;
}
int64_t CerebroBroker::sell_short(const Symbol &symbol, double quantity, double price)
{
    // TODO
    return 0;
}

// 撤单视作特殊的订单, 待撤订单id填写在扩展字段中
int64_t CerebroBroker::cancel_order(int64_t order_id)
{
    // 找出待撤订单
    // 更新订单到待撤销状态
    // matcher 撮合时，遇到待撤订单，则执行撤销，并更新订单

    // 记录到待撤订单列表
    return 0;
}

void CerebroBroker::on_tick(const CerebroTickRecord &record) // 接收tick时间，触发订单撮合
{
    // TODO
    for (auto &o : _orders)
    {
        if (o.status == ORDER_STATUS::REJECTED || o.status == ORDER_STATUS::FILLED)
        {
            continue;
        }

        auto it = _exchg2matcher.find("EXCHANGE");
        if (it == _exchg2matcher.cend())
        {
            // TODO
            LOGE("matcher not found");
            return;
        }
        int rc = it->second->match(o, record);
        if (rc != 0)
        {
            // TODO
            // 处理撮合异常
            LOGE("matche fail, rc=%d %s", rc, o.symbol.c_str());
            return;
        }

        // 通知订单更新
        on_order_update(o);

        if (o.status == ORDER_STATUS::FILLED || o.status == ORDER_STATUS::CANCELLED ||
            o.status == ORDER_STATUS::REJECTED)
        {
            // 从队列移除完成的订单
            // TODO
        }

        if (util::almost_eq(o.unfilled_quantity, 0.0))
        {
            // 全成
            // == 0  撮合完成(仅完全成交才返回0)，将订单从待撮合列表移除
        }
        else
        {
            // 部成
            // TODO
            CerebroOrder suborder = o;
            // 重新生成订单id
            // suborder.order_id
            suborder.quantity = suborder.unfilled_quantity;
            suborder.unfilled_quantity = 0.0;
            suborder.status = ORDER_STATUS::PENDING_NEW;
            suborder.transaction_cost = 0.0;
            suborder.ctime = 0; // 创建时间
            suborder.mtime = 0;
            add_order(suborder);
        }
    }
}