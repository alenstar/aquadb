#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdlib>
#include <cstdint>
#include "httplib/httplib.h"
#include "util/threadpool.h"

namespace mdlink_api {
    struct MarketQuote {
        char            code[16];
        int32_t         date = 0;
        int32_t         time = 0;
        double          open = 0;
        double          high = 0;
        double          low = 0;
        double          close = 0;
        double          last = 0;
        double          high_limit = 0;
        double          low_limit = 0;
        double          pre_close = 0;
        int64_t         volume = 0;
        double          turnover = 0;

        std::vector<double> bid;
        std::vector<double> ask;
        std::vector<int64_t> ask_vol;
        std::vector<int64_t> bid_vol;

        friend std::ostream &operator<<(std::ostream &oss, const MarketQuote &o) 
        {
            oss << o.code <<",";
            oss << o.date <<",";
            oss << o.time <<",";
            oss << o.open <<",";
            oss << o.high <<",";
            oss << o.low <<",";
            oss << o.close <<",";
            oss << o.last <<",";
          
            oss << o.pre_close << ",";
            oss << o.volume << ",";
            for(size_t i = 0; i < 5;++i)
            {
                oss << o.bid[i] << ",";
                oss << o.bid_vol[i] << ",";
                oss << o.ask[i] << ",";
                oss << o.ask_vol[i] << ",";
            }
              oss << o.high_limit <<",";
            oss << o.low_limit;
            return oss;
        }

    };
    typedef std::shared_ptr<MarketQuote> MarketQuotePtr;
    typedef std::function<bool(std::shared_ptr<MarketQuote>)> quotes_callback_t;

    class MdLinkApi {
    public:
        virtual ~MdLinkApi() = default;
        virtual int get_quotes(const std::vector<std::string>& codes,std::function<bool(std::shared_ptr<MarketQuote>)> fn) = 0;
    };
    typedef std::shared_ptr<MdLinkApi> MdLinkApiPtr;


    class MarketDataBasic
    {
        public:
        MarketDataBasic();
        virtual ~MarketDataBasic();
        int get_codes(std::vector<std::string>& codes);
        private:
        httplib::Client* _cli{nullptr};
        std::string _url;
    };

    class MarketDataProvider
    {
        public:
        int init(int size);
        int update_codes();
        int update_quotes();

        bool on_quote(std::shared_ptr<MarketQuote> quote)
        {
            std::cout << *quote << std::endl;
            for(auto it: _quotes_callbacks)
            {
                it.second(quote);
            }
            return true;
        }
        void add_quotes_callback(const std::string& name,std::function<bool(std::shared_ptr<MarketQuote>)> fn)
        {
            _quotes_callbacks[name] = fn;
        }

        void wait_for_shutdown()
        {
            _thpool->WaitAll();
        }

        inline const std::vector<std::string>& get_codes() const {return _codes;}
        private:
        int _pool_size{4};
        std::vector<std::string> _codes;
        std::map<std::string, quotes_callback_t> _quotes_callbacks;
        MarketDataBasic _basic; 
        //std::vector<MdLinkApiPtr> _sina_api;
        //std::vector<MdLinkApiPtr> _tencent_api;
        std::unique_ptr<util::ThreadPool> _thpool;
    };
}
