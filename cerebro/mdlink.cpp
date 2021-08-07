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
        std::cerr << "error=" << err << ",url=" << url << std::endl;
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
    url_ = "/quotes_service/api/json_v2.php/Market_Center.getHQNodeData?node=%s&page=%d&num=%d";
    cli_ = new httplib::Client("http://vip.stock.finance.sina.com.cn");
    cli_->set_keep_alive(false);
    cli_->set_tcp_nodelay(false);
    cli_->set_follow_location(true);
    cli_->set_decompress(true);
    cli_->set_url_encode(false);
    cli_->set_default_headers(
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
    delete cli_;
}
std::string MarketDataBasic::convert_to_inner_code(const std::string& code)
{
    if(util::endswith(code, ".SH"))
    {
        return std::string("sh") + code.substr(0,6);
    }
    else if(util::endswith(code, ".SZ"))
    {
        return std::string("sz") + code.substr(0,6);
    }
    else {
        return code;
    }
}

std::string MarketDataBasic::convert_to_extend_code(const std::string& code)
{
    if(util::startswith(code, "sh"))
    {
        return code.substr(2,6) + ".SH";
    }
    else if(util::startswith(code, "sz"))
    {
        return code.substr(2,6) + ".SZ";
    }
    else {
        return code;
    }
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
        int rc = request(cli_, url.c_str(), out);
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
            snprintf(url_buf, sizeof(url_buf) - 1, url_.c_str(), "sz_a", i, PAGE_SIZE);
            rc = request(cli_, url_buf, body);
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
                LOGE("invalid data:%s", body.c_str());
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
        int rc = request(cli_, url.c_str(), out);
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
            snprintf(url_buf, sizeof(url_buf) - 1, url_.c_str(), "sh_a", i, PAGE_SIZE);
            rc = request(cli_, url_buf, body);
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
                LOGE("invalid data:%s", body.c_str());
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

int MarketDataProvider::init(int size, const std::string& api_name)
{
    if (size > 0)
    {
        pool_size_ = size;
    }
    else 
    {
        pool_size_ = 4;
    }
        if(api_name != "sina" && api_name != "tencent")
        { 
            LOGE("unknown api_name:%s", api_name.c_str());
            return -1;
        }

    api_name_ = api_name;
    thpool_.reset(new util::ThreadPool(pool_size_));

    /*
    for(int i = 0; i < pool_size_; ++i)
    {
        mdlink_api::MdLinkApiPtr api = nullptr;
        if(api_name == "sina")
        { 
        api = std::make_shared<sina_api::SinaApi>();
        }
        else if(api_name == "tencent") {
        api = std::make_shared<tencent_api::TencentApi>();
        }
        else {
            LOGE("unknown api_name:%s", api_name.c_str());
            return -1;
        }
       quotes_api_.push_back(api);
    }
    */
    return 0;
}

MdLinkApiPtr MarketDataProvider::get_mdlink_api()
{
    auto id = std::hash<std::thread::id>()(std::this_thread::get_id());

    util::Mutex::Lock lck(api_mtx_);
    auto it = quotes_api_.find(id); 
    if(it == quotes_api_.end())
    {
        mdlink_api::MdLinkApiPtr api = nullptr;
        if(api_name_ == "sina")
        { 
        api = std::make_shared<sina_api::SinaApi>();
        }
        else if(api_name_ == "tencent") {
        api = std::make_shared<tencent_api::TencentApi>();
        }
        quotes_api_[id] = api;
    }
    return quotes_api_[id];
}


 int MarketDataProvider::update_codes()
 {
     std::vector<std::string> codes;
    int rc = basic_.get_codes(codes);
    if(rc != 0)
    {
        LOGE("get_codes failed, rc=%d", rc);
        return rc;
    }
    LOGD("codes.size=%lu",codes.size());
    if(codes.empty())
    {
        return 0;
    }
    util::RWMutex::WriteLock lck(codes_mtx_);
    codes_ = std::move(codes);
    return 0;
 }

int MarketDataProvider::update_quotes()
{
    size_t num = 30;
    std::vector<std::string> codes;
    std::vector<std::string> mycodes;
    {
    util::RWMutex::ReadLock lck(codes_mtx_);
    mycodes = codes_;
    }
    size_t sz = mycodes.size();
    for(size_t i = 0; i < sz; ++i)
    {
        codes.push_back(mycodes.at(i));
        if(codes.size() == num)
        {
            auto fn = [this,codes](){
                auto api = get_mdlink_api();
                api->get_quotes(codes, [this](MarketQuotePtr quote){
                    this->on_quote(quote);
                    return true;
                } ); 
            };
            thpool_->AddJob(fn);

            codes.clear();
        }
    }

    if(codes.size())
    {
            auto fn = [this,codes](){
                auto api = get_mdlink_api();
                api->get_quotes(codes, [this](MarketQuotePtr quote){
                    this->on_quote(quote);
                    return true;
                } ); 
            };
            thpool_->AddJob(fn);

    }
    return 0;
}

int MarketDataProvider::update_quotes(const std::string& code)
{
    auto c = MarketDataBasic::convert_to_inner_code(code);
            auto fn = [this,c](){
                auto api = get_mdlink_api();
                api->get_quotes({c}, [this](MarketQuotePtr quote){
                    this->on_quote(quote);
                    return true;
                } ); 
            };
            thpool_->AddJob(fn);
    return 0;
}

}