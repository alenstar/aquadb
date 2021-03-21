#include "broker.h"

BrokerBase::BrokerBase() {}

BrokerBase::~BrokerBase() {}

int BrokerBase::init() {
    _omgr = std::make_shared<OrderManager>();
    _pmgr = std::make_shared<PositionManager>();
    return 0;
}

int64_t BrokerBase::buy( const std::string &symbol, int64_t qty, double price ) {
    Order o;
    o.symbol = symbol;
    o.qty    = qty;
    o.price  = price;
    o.side   = OrderSide::BUY;
    if ( util::almost_eq( price, 0.0 ) ) {
        o.type = OrderType::MKT;
    } else {
        o.type = OrderType::LMT;
    }

    auto oe = new_order( o, _timestamp_ms );
    //_omgr->push_order(o);
    _orders[ symbol ].push_back( oe );
    return 0;
}

int64_t BrokerBase::sell( const std::string &symbol, int64_t qty, double price ) {
    Order o;
    o.symbol = symbol;
    o.qty    = qty;
    o.price  = price;
    o.side   = OrderSide::SELL;
    if ( util::almost_eq( price, 0.0 ) ) {
        o.type = OrderType::MKT;
    } else {
        o.type = OrderType::LMT;
    }
    auto oe = new_order( o, _timestamp_ms );
    //_omgr->push_order(o);
    _orders[ symbol ].push_back( oe );
}

void BrokerBase::match( const std::string &symbol, double price, int64_t ts ) {
    auto it = _orders.find( symbol );
    for ( auto e : it->second ) {
        if ( e == nullptr ) {
            continue;
        }
        if ( e->get_order_status() != OrderStatus::NEW ) {
            continue;
        }
        if ( e->get_order_type() == OrderType::MKT ) {
            auto &o      = e->get();
            o.filled_qty = o.qty;
            if ( o.side == OrderSide::BUY ) {
                o.filled_price = price + _slippage * 0.01;
            } else if ( o.side == OrderSide::SELL ) {
                o.filled_price = price - _slippage * 0.01;
            }
            o.filled_time = ts;
        } else if ( e->get_order_type() == OrderType::LMT ) {
            auto &o = e->get();
            if ( o.side == OrderSide::BUY ) {

                if ( o.price > price ) {
                    o.filled_price = o.price;
                } else if ( o.price <= price ) {
                    o.filled_price = price;
                }
            } else if ( o.side == OrderSide::SELL ) {

                if ( o.price > price ) {
                    o.filled_price = price;
                } else if ( o.price <= price ) {
                    o.filled_price = o.price;
                }
            }
        }

        // push order filled event
        // TODO
        update_filled_position( symbol, e->get().filled_qty, e->get().filled_price, e->get().filled_time,
                                e->get().side == OrderSide::BUY );
    }
}

void BrokerBase::match( const std::string &symbol, const sofa::pbrpc::quant::TickRecord &bar ) {
    LOG_INFO( "symbol:" << symbol << " " << bar.ts() << " " << bar.open() << " " << bar.last() << " " << bar.low()
                        << " " << bar.high() << " " << bar.volumn() << " " << bar.amount() << " " << bar.pre_close() );
    auto it = _orders.find( symbol );
    if ( it == _orders.cend() ) {
        return;
    }
    match( symbol, bar.last(), bar.ts() );

    std::vector<OrderExecutorPtr> orders;
    std::vector<OrderExecutorPtr> ordersfilled;
    for ( auto o : it->second ) {
        if ( o->get_order_status() == OrderStatus::NEW ) {
            orders.push_back( o );
        } else if ( o->get_order_status() == OrderStatus::FILLED ) {
            ordersfilled.push_back( o );
        }
    }
    it->second              = std::move( orders );
    _ordersfilled[ symbol ] = std::move( ordersfilled );

    /*
    for(auto e = it->second.begin();e != it->second.end(); ++e)
    {
      auto et = (*e)->match(bar);
      if (et && et->get_order_status() != OrderStatus::NEW)
      {
          _ordersfilled[symbol].push_back(et);
          // push order filled event
          // TODO
          update_filled_position(symbol, et->get().filled_qty, et->get().filled_price, et->get().filled_time,
                                 et->get().side == OrderSide::BUY);
      }
    }
    */
}

PositionPtr BrokerBase::get_position( const std::string &symbol, bool is_long, bool auto_create ) {
    return _pmgr->get_position( symbol, is_long, auto_create );
}
