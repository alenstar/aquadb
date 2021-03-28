#include "ThostStructConvert.h"
#include "cerebro_struct.h"

inline std::string convertFutureSymbol(const char* ins, const char* exchgId, int tradeDate)
{
    size_t sz = std::strlen(ins);
    if(sz<4)
    {
        return (exchgId == nullptr ? std::string(ins)  : (std::string(ins) + "." + exchgId)); 
    }

    std::size_t first_pos = (sz - 1); 
    for(; first_pos >=0; --first_pos)
    {
        if(std::isdigit(ins[first_pos]))
        {
            continue;
        }
        break;
    }
    first_pos+=1;

    // 取出数字部分
    if (first_pos == 0)
    {
        // 没有数字
        return (exchgId == nullptr ? std::string(ins)  : (std::string(ins) + "." + exchgId)); 
    }

    // 仅处理年月只有3位的标的
    if ((sz - first_pos) != 3)
    {

        return (exchgId == nullptr ? std::string(ins)  : (std::string(ins) + "." + exchgId)); 
    }
    // 取出合约分类
    std::string category(ins, first_pos);
    // 转换数字部分为整型,方便补全年份
    int date = std::stoi(ins + first_pos);
    if (date < 1000)
    {
        // 取出交易日的年份
        int t = (tradeDate/10000) - (tradeDate/1000000) * 100; 
        // 年份相差3年以上的加10年
        // 期货合约一般两年以内
        while ((t - (date / 100)) > 3)
        {
            date += 1000;
        }
        char tmp[16] = {0x00};
        sprintf(tmp, "%04d", date);
        return category + tmp + "." + exchgId;
    }
    return (exchgId == nullptr ? std::string(ins)  : (std::string(ins) + "." + exchgId)); 
}

int convet_instrument_record(const CThostFtdcInstrumentField& field, CerebroInstrument& instrument)
{
    instrument.symbol = convertFutureSymbol(field.InstrumentID, field.ExchangeID, util::strto<int>(field.OpenDate));
    instrument.name = field.InstrumentName;
    instrument.exchange = field.ExchangeID;
    instrument.product = field.ProductClass;
    instrument.code = field.InstrumentID;
    instrument.tick_size = field.PriceTick;
    //instrument.maturity_date = field.ExpireDate;
    //instrument.de_listed_date = field.StartDelivDate;
    instrument.contract_multiplier = field.VolumeMultiple;
    instrument.round_lot = 1;
    instrument.min_volume =  field.MinLimitOrderVolume < field.MinMarketOrderVolume ? field.MinLimitOrderVolume : field.MinMarketOrderVolume ;
    instrument.max_volume = field.MaxLimitOrderVolume > field.MaxMarketOrderVolume ? field.MaxMarketOrderVolume: field.MaxLimitOrderVolume;
    instrument.long_margin_rate = field.LongMarginRatio;
    instrument.short_margin_rate = field.ShortMarginRatio;
    return 0;
}

int convet_tick_record(const CThostFtdcDepthMarketDataField& depth, CerebroTickRecord& record)
{
    ///交易日
	record.dt = util::strto<int>(depth.TradingDay);
	///业务日期
	int adt = util::strto<int>(depth.ActionDay);
	///最后修改时间
    int tms = util::strto<int>(util::replace(depth.UpdateTime, ":", "")); 

    if(strcmp(depth.ExchangeID, "DCE") == 0)
    {
        ///交易日
        // 大商所夜盘两个日期都是tradingday
        // record.dt = util::prev_date(record.dt,1);
    }
    else if(strcmp(depth.ExchangeID , "CZCE") == 0)
    {
	    ///业务日期
        // 郑商所日夜盘都是当天日期
        // adt = util::next_date(adt,1);
    }
    // 构造时间
    // build_time(adt, tms, depth.UpdateMillisec);

	///合约代码
	//depth.InstrumentID;
	///交易所代码
	//depth.ExchangeID;
	///合约在交易所的代码
	//depth.ExchangeInstID;
    record.symbol = convertFutureSymbol(depth.InstrumentID, depth.ExchangeID, record.dt);

	///当日均价
	//depth.AveragePrice;

	///最新价
    record.last = depth.LastPrice;
	///上次结算价
	record.prev_settlement = depth.PreSettlementPrice;
	///昨收盘
	record.prev_close = depth.PreClosePrice;
	///昨持仓量
	record.prev_open_interest = depth.PreOpenInterest;
	///今开盘
	record.open =  depth.OpenPrice;
	///最高价
	record.high =  depth.HighestPrice > depth.LastPrice ? depth.HighestPrice : depth.LastPrice;
	///最低价
	record.low = depth.LowestPrice < depth.LastPrice ? depth.LowestPrice : depth.LastPrice;
	///数量
	record.volume = depth.Volume;
	///成交金额
	record.total_turnover = depth.Turnover;
	///持仓量
	record.open_interest = depth.OpenInterest;
	///今收盘
	record.close = depth.LastPrice;
	///本次结算价
    record.settlement =	depth.SettlementPrice;
	///涨停板价
	record.limit_up = depth.UpperLimitPrice;
	///跌停板价
	record.limit_down = depth.LowerLimitPrice;

	///昨虚实度
	//depth.PreDelta;
	///今虚实度
	//depth.CurrDelta;


    do {
	///申买价一
    if(util::almost_eq(depth.BidPrice1, 0.0)) break; 
    record.bids.push_back(depth.BidPrice1);
	///申买价二
    if(util::almost_eq(depth.BidPrice2, 0.0)) break; 
    record.bids.push_back(depth.BidPrice2);
	///申买价三
    if(util::almost_eq(depth.BidPrice3, 0.0)) break; 
    record.bids.push_back(depth.BidPrice3);
	///申买价四
    if(util::almost_eq(depth.BidPrice4, 0.0)) break; 
    record.bids.push_back(depth.BidPrice4);
	///申买价五
    if(util::almost_eq(depth.BidPrice5, 0.0)) break; 
    record.bids.push_back(depth.BidPrice5);
    } while(0);

    do {
	///申买量一
    if(depth.BidPrice1 == 0) break; 
    record.bid_vols.push_back(depth.BidVolume1);
	///申买量二
    if(depth.BidVolume2 == 0) break; 
    record.bid_vols.push_back(depth.BidVolume2);
    ///申买量三
    if(depth.BidVolume3 == 0) break; 
    record.bid_vols.push_back(depth.BidVolume3);
    ///申买量四
    if(depth.BidVolume4 == 0) break; 
    record.bid_vols.push_back(depth.BidVolume4);
	///申买量五
    if(depth.BidVolume5 == 0) break; 
    record.bid_vols.push_back(depth.BidVolume5);
    } while(0);

    do{
	///申卖价一
    if(util::almost_eq(depth.AskPrice1, 0.0)) break; 
    record.asks.push_back(depth.AskPrice1);
	///申卖价二
    if(util::almost_eq(depth.AskPrice2, 0.0)) break; 
    record.asks.push_back(depth.AskPrice2);
	///申卖价三
    if(util::almost_eq(depth.AskPrice3, 0.0)) break; 
    record.asks.push_back(depth.AskPrice3);
	///申卖价四
    if(util::almost_eq(depth.AskPrice4, 0.0)) break; 
    record.asks.push_back(depth.AskPrice4);
	///申卖价五
    if(util::almost_eq(depth.AskPrice5, 0.0)) break; 
    record.asks.push_back(depth.AskPrice5);
    } while(0);

    do{
	///申卖量一
    if(depth.AskVolume1 == 0) break; 
    record.ask_vols.push_back(depth.AskVolume1);
	///申卖量二
    if(depth.AskVolume2 == 0) break; 
    record.ask_vols.push_back(depth.AskVolume2);
	///申卖量三
    if(depth.AskVolume3 == 0) break; 
    record.ask_vols.push_back(depth.AskVolume3);
    ///申卖量四
    if(depth.AskVolume4 == 0) break; 
    record.ask_vols.push_back(depth.AskVolume4);
	///申卖量五
    if(depth.AskVolume5 == 0) break; 
    record.ask_vols.push_back(depth.AskVolume5);
    } while(0);


    return 0;
}

void KLineGenerator::on_kline(const CerebroKlineRecord& record)
{
    // TODO
}

void KLineGenerator::pump_tick(const Symbol& symbol, const CerebroTickRecord& record)
{
    bool newline = false;
    // TODO
    // 获取时间点, 做时间切片
    int minute = static_cast<int>(record.ts/1000) - 1; // 毫秒时间戳转秒
    if(minute % 60 == 0) // 一分钟
    {}

    CerebroKlineRecord& kline = _record;

    if(_last_minute == 0)
    {
        newline = true;
        _last_minute = minute % 60 ;
    }
    else if( minute % 60 != _last_minute) // 切换分钟 
    {
        newline = true;
        _last_minute = minute % 60 ;
    }

    if(newline)
    {
        kline.symbol = record.symbol;
        kline.volume = record.volume - _first_volume;
        kline.open_interest = record.open_interest;
        kline.open = record.last;
        kline.high= record.last;
        kline.low= record.last;
        kline.close= record.last;
    }
    else
    {
         // Filter tick data with older timestamp
        if(record.ts < _last_ts)
        {
            return;
        }
        kline.high = std::max(kline.high, record.last);
        kline.low= std::min(kline.high, record.last);
        kline.close= record.low;
        kline.open_interest = record.open_interest;
        kline.volume = record.volume - _first_volume;
    }
    
}

void KLineGenerator::pump_tick(const CerebroTickRecord& record)
{
    // TODO
}
