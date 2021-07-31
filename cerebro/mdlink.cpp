#include "util/common.h"
#include "util/logdef.h"
#include "nlohmann/json.hpp"

#include "mdlink.h"
#include "sina_api.h"
#include "tencent_api.h"

#define PAGE_SIZE (90)

namespace mdlink_api
{

inline int request(httplib::Client *cli, const char *url, std::string &body)
{
    auto res = cli->Get(url);
    if (res.error() != httplib::Error::Success)
    {
        // TODO
        auto err = res.error();
        std::cerr << "status=" << res->status << ",error=" << err << ",url=" << url << std::endl;
        return static_cast<int>(err);
    }
    else if (res->status == 200)
    {
        // std::cout << ">url=" << url << std::endl;
        // std::cout << ">header=" << util::to_string(res->headers.begin(), res->headers.end())<< std::endl;
        // std::cout << ">body.size=" << res->body.size() <<"\n" << res->body << std::endl;
        body = std::move(res->body);
    }
    else
    {
        // TODO
        auto err = res.error();
        std::cerr << "status=" << res->status << ",error=" << err << ",url=" << url << std::endl;
        return static_cast<int>(err);
    }
    return 0;
}

MarketDataBasic::MarketDataBasic()
{
    // http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getHQNodeData?node=sh_a&page=100&num=15
    _url = "/quotes_service/api/json_v2.php/Market_Center.getHQNodeData?node=%s&page=%d&num=%d";
    _cli = new httplib::Client("http://vip.stock.finance.sina.com.cn");
    _cli->set_keep_alive(true);
    _cli->set_tcp_nodelay(true);
    _cli->set_follow_location(true);
    _cli->set_decompress(true);
    _cli->set_url_encode(false);
    _cli->set_default_headers(
        {{"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
         {"Accept-Language", "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2"},
         {"User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0"},
         {"Upgrade-Insecure-Requests", "1"},
         {"Pragma", "no-cache"},
         //{"Accept-Encoding", "gzip" }
         {"Accept-Encoding", "gzip, deflate"}});
    //  _cli->set_proxy("192.168.53.99", 8888);
}
MarketDataBasic::~MarketDataBasic() {
    delete _cli;
}
int MarketDataBasic::get_codes(std::vector<std::string> &codes)
{
    size_t sha_num = 0;
    size_t sza_num = 0;
    // sza
    do
    {
        std::string url = "/quotes_service/api/json_v2.php/Market_Center.getHQNodeStockCount?node=sz_a";
                std::string out;
        int rc = request(_cli, url.c_str(), out);
        if (rc != 0)
        {
            // TODO
            return rc;
        }
        sscanf(out.c_str(), "\"%lu\"", &sza_num);

        // std::cout << "sza=" << out << std::endl;
        size_t max_page = (sza_num + PAGE_SIZE -1) / PAGE_SIZE;
        for (size_t i = 1; i <= max_page; ++i)
        {
            std::string body;
            char url_buf[256] = {0x0};
            snprintf(url_buf, sizeof(url_buf) - 1, _url.c_str(), "sz_a", i, PAGE_SIZE);
            rc = request(_cli, url_buf, body);
            if (rc != 0)
            {
                // TODO
                return rc;
            }
            // TODO
            if (body.empty())
            {
                break;
            }
            auto js = nlohmann::json::parse(body);
            if(!js.is_array())
            {
                // TODO
                std::cerr << "invalid data:" << body << std::endl;
                return -1;
            }
            for(auto const& a: js)
            {
               codes.push_back(a.at("symbol").get<std::string>());
            }
            if(js.size() < PAGE_SIZE)
            {
                break;
            }
        }
    } while (0);

    // sha
    do
    {
        std::string url = "/quotes_service/api/json_v2.php/Market_Center.getHQNodeStockCount?node=sh_a";
        std::string out;
        int rc = request(_cli, url.c_str(), out);
        if (rc != 0)
        {
            // TODO
            return rc;
        }
        sscanf(out.c_str(), "\"%lu\"", &sha_num);

        //std::cout << "sha=" << out << std::endl;
        size_t max_page = (sha_num + PAGE_SIZE -1 ) / PAGE_SIZE;
        for (size_t i = 1; i <= max_page; ++i)
        {
            std::string body;
            char url_buf[256] = {0x0};
            snprintf(url_buf, sizeof(url_buf) - 1, _url.c_str(), "sh_a", i, PAGE_SIZE);
            rc = request(_cli, url_buf, body);
            if (rc != 0)
            {
                // TODO
                return rc;
            }
            // TODO
            if (body.empty())
            {
                break;
            }
            auto js = nlohmann::json::parse(body);
            if(!js.is_array())
            {
                // TODO
                std::cerr << "invalid data:" << body << std::endl;
                return -1;
            }
            for(auto const& a: js)
            {
               codes.push_back(a.at("symbol").get<std::string>());
            }
            if(js.size() < PAGE_SIZE)
            {
                break;
            }
        }
    } while (0);

    return 0;
}

int MarketDataProvider::init(int size)
{
    if (size > 0)
    {
        _pool_size = size;
    }
    else 
    {
        _pool_size = 4;
    }
_thpool.reset(new util::ThreadPool(_pool_size));
//
//    for(int i = 0; i < _pool_size; ++i)
//    {
//        auto sina = std::make_shared<sina_api::SinaApi>();
//        auto tencent = std::make_shared<tencent_api::TencentApi>();
//        _sina_api.push_back(sina);
//        _tencent_api.push_back(tencent);
//    }
    return 0;
}


 int MarketDataProvider::update_codes()
 {
     std::vector<std::string> codes;
    int rc = _basic.get_codes(codes);
    if(rc != 0)
    {
        LOGE("get_codes failed, rc=%d", rc);
        return rc;
    }
    LOGD("codes.size=%lu",codes.size());
    _codes = std::move(codes);
    return 0;
 }

int MarketDataProvider::update_quotes()
{
    size_t num = 30;
    std::vector<std::string> codes;
    size_t sz = _codes.size();
    for(size_t i = 0; i < sz; ++i)
    {
        codes.push_back(_codes.at(i));
        if(codes.size() == num)
        {
            auto sina = [this,codes](){
                sina_api::SinaApi sina;
                sina.get_quotes(codes, [this](std::shared_ptr<MarketQuote> quote){
                    this->on_quote(quote);
                    return true;
                } ); 
            };
            _thpool->AddJob(sina);

            auto tencent = [this,codes](){
                tencent_api::TencentApi tencent;
                tencent.get_quotes(codes, [this](std::shared_ptr<MarketQuote> quote){
                    this->on_quote(quote);
                    return true;
                } ); 
            };
            _thpool->AddJob(tencent);

            codes.clear();
        }
    }

    if(codes.size())
    {
            auto sina = [this,codes](){
                sina_api::SinaApi sina;
                sina.get_quotes(codes, [this](std::shared_ptr<MarketQuote> quote){
                    this->on_quote(quote);
                    return true;
                } ); 
            };
            _thpool->AddJob(sina);

            auto tencent = [this,codes](){
                tencent_api::TencentApi tencent;
                tencent.get_quotes(codes, [this](std::shared_ptr<MarketQuote> quote){
                    this->on_quote(quote);
                    return true;
                } ); 
            };
            _thpool->AddJob(tencent);
    }
    return 0;
}

}