#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include "httplib/httplib.h"
#include "util/threadpool.h"
#include "util/mutex.h"

#include "cerebro_struct.h"


namespace mdlink_api {
    //typedef std::shared_ptr<MarketQuote> MarketQuotePtr;
    typedef std::shared_ptr<CerebroTickRecord> MarketQuotePtr;
    typedef std::function<bool(MarketQuotePtr)> quotes_callback_t;

    template<typename... Ts>
    std::shared_ptr<CerebroTickRecord> make_quote_ptr(Ts&&... params)
    {
        return std::shared_ptr<CerebroTickRecord>(new CerebroTickRecord(std::forward<Ts>(params)...));
    }



    class MdLinkApi {
    public:
        virtual ~MdLinkApi() = default;
        virtual int get_quotes(const std::vector<std::string>& codes,std::function<bool(MarketQuotePtr)> fn) = 0;
    };
    typedef std::shared_ptr<MdLinkApi> MdLinkApiPtr;


    class MarketDataBasic
    {
        public:
        MarketDataBasic();
        virtual ~MarketDataBasic();
        int get_codes(std::vector<std::string>& codes);

        static std::string convert_to_inner_code(const std::string& code);
        static std::string convert_to_extend_code(const std::string& code);

        private:
        httplib::Client* cli_{nullptr};
        std::string url_;
    };

    class MarketDataProvider
    {
        public:
        MarketDataProvider() = default;
        ~MarketDataProvider() = default;

        int init(int size, const std::string& api_name = "sina");
        int update_codes();
        int update_quotes();
        int update_quotes(const std::string& code);
        inline void set_codes(const std::vector<std::string> & codes) {
            util::RWMutex::WriteLock lck(codes_mtx_);
            codes_ = std::move(codes);
        }

        bool on_quote(MarketQuotePtr quote)
        {
                        // TODO
            if (!std::isnormal(quote->last))
            {
                // 无效行情
                LOGW("invlaid tick: last price invalid, %f, %s", quote->last, quote->symbol.c_str());
                return true;
            }
            {
                util::RWMutex::WriteLock lck(quotes_mtx_);
                auto it = quotes_.find(quote->symbol);
                if(it == quotes_.end())
                {
                    quotes_[quote->symbol] = quote;
                }
                else {
                    if(quote->ts <= it->second->ts)
                    {
                        // 剔除旧行情
                        return true;
                    }
                }
            }
            if(quotes_callback_) {
                return quotes_callback_(quote);
            }
            return true;
        }
        void set_quotes_callback(std::function<bool(MarketQuotePtr)> fn)
        {
            quotes_callback_ = fn;
        }

        void clear()
        {
                util::RWMutex::WriteLock lck(quotes_mtx_);
                quotes_.clear();
        }

        void wait_for_shutdown()
        {
            thpool_->WaitAll();
        }

        inline std::vector<std::string> get_codes() {
            util::RWMutex::ReadLock lck(codes_mtx_);
            std::vector<std::string> codes = codes_;
            return codes;
            }
        protected:
            MdLinkApiPtr get_mdlink_api();

        private:
        int pool_size_{4};
        std::string api_name_{"sina"};

        std::vector<std::string> codes_;
        util::RWMutex codes_mtx_;

        std::map<std::string, MarketQuotePtr> quotes_;
        util::RWMutex quotes_mtx_;

        quotes_callback_t quotes_callback_;
        MarketDataBasic basic_; 

        //std::vector<MdLinkApiPtr> quotes_api_;
        std::map<size_t,MdLinkApiPtr> quotes_api_;
        util::Mutex api_mtx_;

        std::unique_ptr<util::ThreadPool> thpool_;
    };
}
