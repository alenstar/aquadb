#include "cerebro_broker.h"
#include "util/common.h"
#include <algorithm>


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
    case ORDER_ACTION::DIVIDEND:
    case ORDER_ACTION::DEPOSIT:
    case ORDER_ACTION::WITHDRAW:
        return true;
    default:
        break;
    }
    return false;
}

CerebroPositionTracker::CerebroPositionTracker(const Symbol &symbol, POSITION_DIRECTION direction, int market_tplus)
{
    _position.symbol = symbol;
    _position.direction = direction;
    _tplus = market_tplus;
}

void CerebroPositionTracker::buy(double quantity, double price, double commission)
{
    if(util::almost_zero(quantity))
    {
        return;
    }
    // TODO
    if (util::almost_eq(_position.quantity, 0.0))
    {
        // new position
        _position.avg_price = price;
        _position.quantity = quantity;
        _position.commissions = commission;
        if(_tplus == 0){
            _position.closable += quantity;
        }
    }
    else
    {
        _position.avg_price =
            (_position.avg_price * _position.quantity + quantity * price) / (_position.quantity + quantity);
        _position.quantity += quantity;
        _position.commissions += commission;
        if(_tplus == 0){
            _position.closable += quantity;
        }
    }
}
void CerebroPositionTracker::sell(double quantity, double price, double commission)
{
    if(util::almost_zero(quantity))
    {
        return;
    }
    // TODO
    if (util::almost_eq(_position.quantity, 0.0))
    {
        // 没有持仓, 反向开空？
        // new position
        //_position.avg_price = price;
        //_position.quantity = quantity * (-1);
        //_position.commissions = commission;
        //if(_tplus == 0){
        //    _position.closable += quantity;
        //}
        //没持仓了
        throw std::logic_error("There is no position");
        return;
    }
    else
    {
        // 交易的持仓市值(持仓均价计算)
        double mv = _position.avg_price * quantity;
        _position.avg_price =
            (_position.avg_price * _position.quantity - quantity * price) / (_position.quantity - quantity);
        _position.quantity -= quantity;
        _position.commissions += commission;
        //if(_tplus == 0){
            _position.closable -= quantity;
        //}
        
        // 交易盈亏(卖出的市值-持仓市值-手续费)
        if(util::almost_zero(mv)){
            _position.trading_pnl -= commission;
        } else {
            _position.trading_pnl += quantity * price - mv - commission;
        }
        // 持仓盈亏随行情更新，没有持仓时清空
        if(util::almost_zero(_position.quantity)) {
            // 没持仓了，持仓盈亏为0
            _position.position_pnl = 0;
        }
        // 更新Pnl
        // 平均手续费
        //auto avg_cost = _position.commissions / _position.quantity;
        // 差价
        //auto diff_price = record.close - _position.avg_price;
        //_position.position_pnl = (diff_price - avg_cost) * _position.quantity;

    }
}

void CerebroPositionTracker::buy_to_cover(double quantity, double price, double commission)
{
    // TODO
    // 买平
    if (util::almost_eq(_position.quantity, 0.0))
    {
        // new position
        _position.avg_price = price;
        _position.quantity = quantity;
        _position.commissions = commission;
        if(_tplus == 0){
            _position.closable += quantity;
        }
    }
    else
    {
        _position.avg_price =
            (_position.avg_price * _position.quantity + quantity * price) / (_position.quantity + quantity);
        _position.quantity += quantity;
        _position.commissions += commission;
        if(_tplus == 0){
            _position.closable += quantity;
        }
    }
}
void CerebroPositionTracker::sell_short(double quantity, double price, double commission)
{
    // TODO
    //  买空
    if (util::almost_eq(_position.quantity, 0.0))
    {
        // new position
        _position.avg_price = price;
        _position.quantity = quantity;
        _position.commissions = commission;
        if(_tplus == 0){
            _position.closable += quantity;
        }
    }
    else
    {
        _position.avg_price =
            (_position.avg_price * _position.quantity + quantity * price) / (_position.quantity + quantity);
        _position.quantity += quantity;
        _position.commissions += commission;
        if(_tplus == 0){
            _position.closable += quantity;
        }
    }
}

void CerebroPositionTracker::update(const CerebroTickRecord& record)
{
    if(util::almost_zero(_position.quantity))
    {
        return;
    }
    // 平均手续费
    auto avg_cost = _position.commissions / _position.quantity;
    // 差价
    auto diff_price = record.last - _position.avg_price;
    _position.position_pnl = (diff_price - avg_cost) * _position.quantity;
}

void CerebroPositionTracker::update(const CerebroKlineRecord& record)
{
    if(util::almost_zero(_position.quantity))
    {
        return;
    }
    // 平均手续费
    auto avg_cost = _position.commissions / _position.quantity;
    // 差价
    auto diff_price = record.close - _position.avg_price;
    _position.position_pnl = (diff_price - avg_cost) * _position.quantity;
}



// 撮合器， 一个交易所一个撮合器， 内部再按品种分不同处理模块
CerebroMatcher::CerebroMatcher(CerebroBroker *broker) : _broker(broker) {}
CerebroMatcher::~CerebroMatcher() {}

bool CerebroMatcher::is_over_limit_up(CerebroOrder &order, const CerebroTickRecord &tick)
{
    // 不设涨停
    if(util::almost_zero(tick.limit_up) || std::isnan(tick.limit_up))
    {
        return false;
    }
    // 涨停
    if(util::almost_ge(tick.last, tick.limit_up, price_unit(tick.symbol))){
        return true;
    }
    if(util::almost_zero(order.price, price_unit(tick.symbol)))
    {
        return false;
    }
    // 委托价超出涨停价
    if(util::almost_gt(order.price, tick.limit_up, price_unit(tick.symbol)))
    {
        return true;
    }
    return false;
}

bool CerebroMatcher::is_over_limit_down(CerebroOrder &order, const CerebroTickRecord &tick)
{
    // 不设跌停
    if(util::almost_zero(tick.limit_down) || std::isnan(tick.limit_down))
    {
        return false;
    }
    // 跌停
    if(util::almost_le(tick.last, tick.limit_down, price_unit(tick.symbol))){
        return true;
    }
    if(util::almost_zero(order.price, price_unit(tick.symbol)))
    {
        return false;
    }
    // 委托价超出跌停价
    if(util::almost_lt(order.price, tick.limit_down, price_unit(tick.symbol)))
    {
        return true;
    }
    return false;
}

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
        // 涨停不允许买入
        if(is_over_limit_up(order, tick))
        {
            LOGW("over limit up order: %s %d, order_price: %f, last_price: %f", order.symbol.c_str(), tick.dt, order.price, tick.last);
            order.status = ORDER_STATUS::REJECTED;
            order.reason = "over limit up";
            return 0;
        }
        // auto tacker = _broker->get_long_tracker(order.symbol,true);
            auto tota_price = (order.price + _broker->get_slippage() * price_unit(order.symbol)) * order.quantity +  _broker->calc_commission(order);
            // TODO
            // 冻结资金，后面使的冻结的资金来操作
            if(util::almost_gt(tota_price, _broker->get_cash()))
            {
                LOGW("invlaid order: Insufficient available funds, cash: %f, price: %f ,%s", _broker->get_cash(), tota_price, order.symbol.c_str());
            order.status = ORDER_STATUS::REJECTED;
                order.reason = " Insufficient available funds";
                return 0;
            }

            if (util::almost_lt(order.price, tick.last))
            {
                LOGD("price not match tick.last=%f order.price=%f %s", tick.last, order.price, order.symbol.c_str());
                return 0;
            }
            order.filled_price = tick.last + _broker->get_slippage() * price_unit(order.symbol);
            // 成交量处理
            order.filled_quantity = std::min(tick.ask_vols.at(0), order.quantity);
            //order.filled_quantity = order.quantity;
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            // 订单自带手续费时，不计算手续费 (子单自带手续费)
            if(util::almost_eq(order.transaction_cost, 0.0)) {
            order.transaction_cost = _broker->calc_commission(order);
            }
            order.status = ORDER_STATUS::FILLED;

            if (util::almost_ne(order.quantity, order.filled_quantity))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                // 手续费分摊
                order.transaction_cost = order.transaction_cost * (order.filled_quantity/order.quantity);
            }
            _broker->current_account()->transform_cash_to_market_value(order.filled_price * order.filled_quantity, order.transaction_cost);
            return 0;
    }
    else if (order.action == ORDER_ACTION::SELL)
    {
        // 跌停不允许卖出
        if(is_over_limit_down(order, tick))
        {
            LOGW("over limit down order: %s %d, order_price: %f, last_price: %f", order.symbol.c_str(),tick.dt, order.price, tick.last);
            order.status = ORDER_STATUS::REJECTED;
            order.reason = "over limit down";
            return 0;
        }
        auto tracker = _broker->get_long_tracker(order.symbol,true);
        if(util::almost_gt(order.quantity, tracker->current_closable()))
        {
            LOGW("invlaid order,Can close the position insufficient: %s, closable:%f, order.quantity:%f", order.symbol.c_str(), tracker->current_closable(),order.quantity);
            order.status = ORDER_STATUS::REJECTED;
            order.reason = "Can close the position insufficient";
            return 0;
        }
            if (util::almost_gt(order.price, tick.last))
            {
                LOGD("price not match tick.last=%f order.price=%f %s", tick.last, order.price, order.symbol.c_str());
                return 0;
            }
            order.filled_price = tick.bids.at(0) + _broker->get_slippage() * price_unit(order.symbol);
            // 处理成交量
            order.filled_quantity = std::min(tick.bid_vols.at(0), order.quantity);
            //order.filled_quantity = order.quantity;
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            if(util::almost_eq(order.transaction_cost, 0.0)) {
            order.transaction_cost = _broker->calc_commission(order);
            }
            order.status = ORDER_STATUS::FILLED;
            order.last_position_price = tracker->current_position().avg_price;
            if (util::almost_ne(order.quantity, order.filled_quantity))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                // 手续费分摊
                order.transaction_cost = order.transaction_cost * (order.filled_quantity/order.quantity);
            }
            _broker->current_account()->transform_market_value_to_cash(order.filled_price * order.filled_quantity, order.transaction_cost);
            return 0;
    }
    //else if (order.action == ORDER_ACTION::BUY_TO_COVER) {}
    //else if (order.action == ORDER_ACTION::SELL_SHORT) {}
    else
    {
        // TODO
        LOGE("unknown order.action=%d", static_cast<int>(order.action));
        order.status = ORDER_STATUS::REJECTED;
        order.reason = "unknown order action";
    }
    return 0;
}
int CerebroMatcher::process_market_order(CerebroOrder &order, const CerebroTickRecord &tick)
{
    if (order.action == ORDER_ACTION::BUY)
    {
                // 涨停不允许买入
        if(is_over_limit_up(order, tick))
        {
            LOGW("over limit up order: %s %d, order_price: %f, last_price: %f", order.symbol.c_str(), tick.dt, order.price, tick.last);
            order.status = ORDER_STATUS::REJECTED;
            order.reason = "over limit up";
            return 0;
        }
            auto tota_price = (tick.last + _broker->get_slippage() * price_unit(order.symbol)) * order.quantity +  _broker->calc_commission(order);
            if(util::almost_gt(tota_price, _broker->get_cash()))
            {
                LOGW("invlaid order: Insufficient available funds, cash: %f, price: %f ,%s", _broker->get_cash(), tota_price, order.symbol.c_str());
                order.status = ORDER_STATUS::REJECTED;
                order.reason = " Insufficient available funds";
                return 0;
            }

            order.filled_price = tick.last + _broker->get_slippage() * price_unit(order.symbol);
            order.filled_quantity = std::min(tick.ask_vols.at(0), order.quantity);
            //order.filled_quantity = order.quantity;
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            if(util::almost_eq(order.transaction_cost, 0.0)) {
            order.transaction_cost = _broker->calc_commission(order);
            }
            order.status = ORDER_STATUS::FILLED;

            if (util::almost_ne(order.quantity, order.filled_quantity))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                // 手续费分摊
                order.transaction_cost = order.transaction_cost * (order.filled_quantity/order.quantity);
            }
            _broker->current_account()->transform_cash_to_market_value(order.filled_price * order.filled_quantity, order.transaction_cost);
            return 0;
    }
    else if (order.action == ORDER_ACTION::SELL)
    {
                // 跌停不允许卖出
        if(is_over_limit_down(order, tick))
        {
            LOGW("over limit down order: %s %d, order_price: %f, last_price: %f", order.symbol.c_str(), tick.dt, order.price, tick.last);
            order.status = ORDER_STATUS::REJECTED;
            order.reason = "over limit down";
            return 0;
        }
        auto tracker = _broker->get_long_tracker(order.symbol,true);
        if(util::almost_gt(order.quantity, tracker->current_closable()))
        {
            LOGW("invlaid order,Can close the position insufficient: %s, closable:%f, order.quantity:%f", order.symbol.c_str(), tracker->current_closable(),order.quantity);
            order.status = ORDER_STATUS::REJECTED;
            order.reason = "Can close the position insufficient";
            return 0;
        }
            order.filled_price = tick.last + _broker->get_slippage() * price_unit(order.symbol);
            order.filled_quantity = std::min(tick.bid_vols.at(0), order.quantity);
            //order.filled_quantity = order.quantity;
            order.mtime = tick.ts;
            order.trading_date = tick.dt;
            if(util::almost_eq(order.transaction_cost, 0.0)) {
                order.transaction_cost = _broker->calc_commission(order);
            }
            order.status = ORDER_STATUS::FILLED;
            order.last_position_price = tracker->current_position().avg_price;
            if (util::almost_ne(order.quantity, order.filled_quantity))
            {
                // 记录未成交量
                order.unfilled_quantity = order.quantity - order.filled_quantity;
                // 手续费分摊
                order.transaction_cost = order.transaction_cost * (order.filled_quantity/order.quantity);
            }
            _broker->current_account()->transform_market_value_to_cash(order.filled_price * order.filled_quantity, order.transaction_cost);
            return 0;
    }
    //else if (order.action == ORDER_ACTION::BUY_TO_COVER) {}
    //else if (order.action == ORDER_ACTION::SELL_SHORT) {}
    else
    {
        // TODO
        LOGE("unknown order.action=%d %s", static_cast<int>(order.action), order.symbol.c_str());
        order.status = ORDER_STATUS::REJECTED;
        order.reason = "unknown order action";
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
        LOGE("unknown order.type=%d", static_cast<int>(order.type));
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
        LOGE("unknown order.type=%d", static_cast<int>(order.type));
    }

    // 无效订单，标记，并更新订单
    // 成交,更新订单, broker来更新
    // do_order_update(order);
    return -1;
}


int CerebroBroker::init(const CerebroConfig& conf)
{
    _account = std::make_shared<CerebroAccountWrap>(conf.aid, conf.name);
    _account->set_cash(conf.cash);
    _account->set_commission_rate(conf.comm_rate);
    _account->set_slippage(conf.slippage);
    return 0;
}

int CerebroBroker::init(CerebroAccountWrapPtr account)
{
    _account = account;

    // 帐号是初次初始化状态时,为保证订单数据完整性，将帐号资金入金，转换成以完成订单
    //if(_account->is_initial())
    //{
    //    // 初次入金，入金订单直接入完成单队列(不重技计算份额)
    //    deposit_cash(_account->get_cash(), true);
    //}
    return 0;
}

int CerebroBroker::process_special_order(const CerebroOrder &order)
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
    case ORDER_ACTION::DIVIDEND:
    // 分红送股(执行前市不变),增加持仓和现金
    {
        auto tracker = get_long_tracker(order.symbol,true);
        // 增加持仓
        if(!util::almost_zero(order.quantity)) {
        tracker->buy(order.quantity, 0.0, order.transaction_cost);
        }
        // 增加现金
        if(!util::almost_zero(order.quantity)) {
        _account->add_cash(order.price, order.transaction_cost);
        }
        return 0;
    }
    // TODO
    case ORDER_ACTION::DEPOSIT:
    // 入金
    return _account->deposit_cash(order.price * order.quantity);
    case ORDER_ACTION::WITHDRAW:
    // 出金
    return _account->withdraw_cash(order.price * order.quantity);
    default:
    // ERROR
        break;
    }
    return 0;
}

    // 入金
int64_t CerebroBroker::deposit_cash(double cash, bool isfinish)
{
    CerebroOrder order;
    order.order_id = _order_id_gen();
    order.symbol = "Deposit Funds";
    order.ctime = now_ms();
    order.mtime = now_ms();
    order.price = cash;
    order.quantity = 1;
    order.status = isfinish ? ORDER_STATUS::FILLED : ORDER_STATUS::PENDING_NEW;
    order.type = ORDER_TYPE::MARKET;
    order.action = ORDER_ACTION::DEPOSIT;
    commit_order(order);
    return order.order_id;
}
// 出金
int64_t CerebroBroker::withdraw_cash(double cash)
{
    CerebroOrder order;
    order.order_id = _order_id_gen();
    order.symbol = "Withdraw Funds";
    order.ctime = now_ms();
    order.mtime = now_ms();
    order.price = cash;
    order.quantity = 1;
    order.status = ORDER_STATUS::PENDING_NEW;
    order.type = ORDER_TYPE::MARKET;
    order.action = ORDER_ACTION::WITHDRAW;
    commit_order(order);
    return order.order_id;
}

void CerebroBroker::commit_order(const CerebroOrder &order) 
{
    if(order.status == ORDER_STATUS::PENDING_NEW)
    {
    if(!is_special_order(order))
    {
        // 非特殊订单进撮合队列
        _unfill_orders[order.symbol].push_back(order); 
    }
    else 
    {
        // 处理特殊订单， 例如： 分红送红股，入金等
        CerebroOrder co = order; 
        int rc = process_special_order(co);
        if(rc != 0)
        {
            co.ctime = now_ms();
            co.status = ORDER_STATUS::REJECTED;
            co.trading_date = now_date();
        }
        else 
        {
            co.ctime = now_ms();
            co.status = ORDER_STATUS::FILLED;
            co.trading_date = now_date();
        }
        _filled_orders.push_back(co);
    }
    }
    else 
    {
        // 非新单直接进完成队列
        _filled_orders.push_back(order);
    }
}

void CerebroBroker::on_order_update(const CerebroOrder &order)
{
    if (is_special_order(order))
    {
        // 特殊订单直接在提交是处理完，不进入撮合队列
        // 处理特殊订单， 例如： 分红送红股，出入金等
        //int rc = process_special_order(order);
        //if(rc != 0)
        //{
        //    // TODO
        //}
        //else 
        //{
        //    // TODO
        //}
        return;
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
        auto tracker = get_long_tracker(order.symbol,true);
        // 多仓方向
        if (order.action == ORDER_ACTION::BUY)
        {
            tracker->buy(order.filled_quantity, order.filled_price, order.transaction_cost);
        }
        else if (order.action == ORDER_ACTION::SELL)
        {
            tracker->sell(order.filled_quantity, order.filled_price, order.transaction_cost);
        }
        else
        {
            // TODO
        }
    }
    else if (order_longshort(order) == POSITION_DIRECTION::SHORT)
    {
        auto tracker = get_long_tracker(order.symbol,true);
        // 空仓方向
        if (order.action == ORDER_ACTION::BUY)
        {
            tracker->buy_to_cover(order.filled_quantity, order.filled_price, order.transaction_cost);
        }
        else if (order.action == ORDER_ACTION::SELL)
        {
            tracker->sell_short(order.filled_quantity, order.filled_price, order.transaction_cost);
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
    if(util::almost_zero(quantity))
    {
        // 无效委托 
        return -1;
    }
    CerebroOrder order;
    order.order_id = _order_id_gen();
    order.symbol = symbol;
    order.ctime = now_ms();
    order.mtime = now_ms();
    order.price = price;
    order.quantity = quantity;
    order.status = ORDER_STATUS::PENDING_NEW;
    order.type = price == 0.0 ? ORDER_TYPE::MARKET : ORDER_TYPE::LIMIT;
    order.action = ORDER_ACTION::BUY;
    // TODO
    commit_order(order);
    return order.order_id;
}
int64_t CerebroBroker::sell(const Symbol &symbol, double quantity, double price)
{
    if(util::almost_zero(quantity))
    {
        // 无效委托 
        return -1;
    }
    CerebroOrder order;
    order.order_id = _order_id_gen();
    order.symbol = symbol;
    order.ctime = now_ms();
    order.mtime = now_ms();
    order.price = price;
    order.quantity = quantity;
    order.status = ORDER_STATUS::PENDING_NEW;
    order.type = price == 0.0 ? ORDER_TYPE::MARKET : ORDER_TYPE::LIMIT;
    order.action = ORDER_ACTION::SELL;
    // TODO
    commit_order(order);
    return order.order_id;
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

 int64_t CerebroBroker::dividend_order(const Symbol &symbol, double quantity, double price)
 {
    CerebroOrder order;
    order.order_id = _order_id_gen();
    order.symbol = symbol;
    order.ctime = now_ms();
    order.mtime = now_ms();
    order.price = price;
    order.quantity = quantity;
    order.status = ORDER_STATUS::PENDING_NEW;
    order.type = price == 0.0 ? ORDER_TYPE::MARKET : ORDER_TYPE::LIMIT;
    order.action = ORDER_ACTION::DIVIDEND;
    // TODO
    commit_order(order);
    return order.order_id;
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

void CerebroBroker::process_normal_order(const CerebroTickRecord &record)
{
    auto it = _unfill_orders.find(record.symbol);
    if (it == _unfill_orders.end())
    {
        // not found order for this symbol
        return;
    }

    std::set<int64_t> oids; // 记录已成订单id
    std::vector<CerebroOrder> neworders; // 新订单，部成时产生的子单
    // TODO
    auto & unfill_orders = it->second;
    for (auto &o : unfill_orders)
    {
        if (o.status == ORDER_STATUS::REJECTED || o.status == ORDER_STATUS::CANCELLED || o.status == ORDER_STATUS::FILLED)
        {
            _filled_orders.push_back(o);
            continue;
        }
        std::string exchg;
        {
            auto pos = o.symbol.find('.');
            exchg = o.symbol.substr(pos + 1);
        }
        auto it = _exchg2matcher.find(exchg);
        if (it == _exchg2matcher.cend())
        {
            // TODO
            LOGE("matcher not found: %s, symbol=%s", exchg.c_str(), o.symbol.c_str());
            continue;
        }
        int rc = it->second->match(o, record);
        if (rc != 0)
        {
            // TODO
            // 处理撮合异常
            LOGE("matche fail, rc=%d, symbol=%s", rc, o.symbol.c_str());
            continue;
        }

        o.mtime = now_ms();
        // 通知订单更新
        on_order_update(o);
        if(_order_update_callback && o.status != ORDER_STATUS::PENDING_NEW){
            _order_update_callback(o);
        }

        if (o.status == ORDER_STATUS::FILLED || o.status == ORDER_STATUS::CANCELLED ||
            o.status == ORDER_STATUS::REJECTED)
        {
            // 添加到完成列表,后续从队列移除完成的订单
            oids.insert(o.order_id);
            // 存入已成队列
            _filled_orders.push_back(o);
        }

        if (util::almost_ne(o.unfilled_quantity, 0.0))
        {
            // 部成
            CerebroOrder suborder;
            suborder.symbol = o.symbol;
            suborder.action = o.action;
            // 重新生成订单id
            suborder.order_id = _order_id_gen();
            suborder.type = o.type;
            suborder.status =  ORDER_STATUS::PENDING_NEW;
            suborder.price = o.price;
            suborder.quantity = o.unfilled_quantity;
            suborder.unfilled_quantity = 0.0;
            suborder.last_position_price = 0.0;
            // 分摊手续费
            suborder.transaction_cost = (o.transaction_cost/(o.filled_quantity/o.quantity)) * (1 - o.filled_quantity/o.quantity);
            suborder.trading_date = 0;
            suborder.reason.clear();
            suborder.ctime = now_ms(); // 创建时间
            suborder.mtime = now_ms();
            suborder.extra["primary_order_id"] = std::to_string(o.order_id);
            neworders.emplace_back(suborder);
            //commit_order(suborder);
        }
    }
    // 移除已成订单
    unfill_orders.erase(std::remove_if(unfill_orders.begin(), unfill_orders.end(), [](CerebroOrder& o){ 
        return (o.status == ORDER_STATUS::REJECTED || o.status == ORDER_STATUS::CANCELLED || o.status == ORDER_STATUS::FILLED);}), unfill_orders.end());
    for(auto & o: neworders)
    {
        if(_order_update_callback){
            _order_update_callback(o);
        }

        // 部分成交的订单再入未成队列
        unfill_orders.push_back(o);
    }
}

double CerebroBroker::positions_market_value()
{
    double value = 0.0;
    for(auto const& p: _lpositions)
    {
        if(p.second->empty())
        {
            continue;
        }
        value += p.second->market_value();
    }
    for(auto const& p: _spositions)
    {
        if(p.second->empty())
        {
            continue;
        }
        value += p.second->market_value();
    }
    return value;
}

double CerebroBroker::current_positions_pnl()
{
    double value = 0.0;
    for(auto const& p: _lpositions)
    {
        value += p.second->current_position_pnl();
    }
    for(auto const& p: _spositions)
    {
        value += p.second->current_position_pnl();
    }
    return value;
}


void CerebroBroker::do_position_update(const CerebroTickRecord &record)
{
    // TODO
    auto lp = _lpositions.find(record.symbol);
    if(lp != _lpositions.cend())
    {
        lp->second->update(record);
    }
    auto sp = _spositions.find(record.symbol);
    if(sp != _spositions.cend())
    {
        sp->second->update(record);
    }
}

void CerebroBroker::on_tick(const CerebroTickRecord &record) // 接收tick数据，触发订单撮合
{
    // 更新持仓盈亏等
    if(has_position(record.symbol)) do_position_update(record);
    // 处理订单
    process_normal_order(record);
    // 撮合后再次 更新持仓盈亏等
    do_position_update(record);
    // 更新账户盈亏市值等
    // 平仓后没有持仓应该清除对应市值，有持仓的更新市值和盈亏
    {
        LOGD("symbol:%s pnl:%f, market_value:%f", record.symbol.c_str(), current_positions_pnl(),positions_market_value());
        _account->update_daily_pnl(current_positions_pnl());
        _account->update_market_value(positions_market_value());
    }
}

int CerebroBroker::current_orders(std::vector<CerebroOrder>& orders)
{
    orders = _filled_orders;
    for(auto const& uo: _unfill_orders)
    {
        for(auto const& o: uo.second) {
            orders.push_back(o);
        }
    }
    return 0;
}

int CerebroBroker::current_positions(std::vector<CerebroPosition>& positions)
{
    for(auto const& p: _lpositions)
    {
        positions.push_back(p.second->current_position());
    }
    for(auto const& p: _spositions)
    {
        positions.push_back(p.second->current_position());
    }
    return 0;
}


int CerebroBroker::settle(const std::vector<CerebroKlineRecord>& records)
{
    for(auto const& record: records)
    {
        // 获取收盘行情来结算
        auto l = get_long_tracker(record.symbol,false);
        if(l)
        {
            l->settle(record);
            LOGD("long symbol:%s, closable:%f,quantity:%f", record.symbol.c_str(), l->current_position().closable, l->current_position().quantity);
        }
        auto s = get_short_tracker(record.symbol,false);
        if(s)
        {
            LOGD("short symbol:%s, closable:%f,quantity:%f", record.symbol.c_str(), l->current_position().closable, l->current_position().quantity);
            s->settle(record);
        }
    }

    // 更新账户收益
    {
        _account->update_daily_pnl(current_positions_pnl());
        //LOGD("symbol:%s pnl:%f, market_value:%f", record.symbol.c_str(), current_positions_pnl(),positions_market_value());
        _account->update_market_value(positions_market_value());
    }

    // 处理除权除息
    // TODO
    
    // 结算数据持久化
    // TODO
    // 每天持仓数据持久化
    // TODO

    // 清理空仓的tracker
    auto trackers = get_all_tracker();
    for(auto const& t: trackers)
    {
        remove_tracker(t->symbol(), t->direction());
    }

    // 当日订单持久化
    // 当日持仓持久化
    // 当日账户持久化
    return 0;
}