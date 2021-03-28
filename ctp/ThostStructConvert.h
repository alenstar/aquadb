#include <iostream>

#include "ThostFtdcUserApiStruct.h"
#include "cerebro_struct.h"

int convet_tick_record(const CThostFtdcDepthMarketDataField& depth, CerebroTickRecord& record);
int convet_instrument_record(const CThostFtdcInstrumentField& field, CerebroInstrument& instrument);

// 一个合约一个生成器
class KLineGenerator
{
    public:
    KLineGenerator();
    ~KLineGenerator();

    // 输入tick数据
    void pump_tick(const CerebroTickRecord& record);
    void pump_tick(const Symbol& symbol, const CerebroTickRecord& record);

    // k线生成后回调该函数
    void on_kline(const CerebroKlineRecord& record);
    private:
    CerebroKlineRecord _record;
    int _period_interval = 0; // 60 一分钟K 
    double _first_volume = 0;// 记录起始成交量
    bool _is_first = true;
    int64_t _last_ts;
    int _last_minute = 0;
};