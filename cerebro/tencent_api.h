#pragma once


#include <memory>
#include <string>
#include <vector>
#include "mdlink.h"
#include "httplib/httplib.h"

namespace tencent_api {

    using namespace std;

    class TencentApi: public mdlink_api::MdLinkApi {
    public:
        TencentApi(int page_size = 15);
        virtual ~TencentApi();

        int get_quotes_by_url(const std::string& url, std::function<bool(mdlink_api::MarketQuotePtr)> fn) ;

        int get_quotes(const std::vector<std::string>& codes, std::function<bool(mdlink_api::MarketQuotePtr)> fn) override;

        static std::string convert_to_inner_code(const std::string& code);
        static std::string convert_to_extend_code(const std::string& code);
        //static std::string convert_to_extend_code(const char* code) {
        //    return convert_to_extend_code(std::string(code));
        //}
    private:
        size_t _page_size {15};
         httplib::Client* _cli {nullptr};
        //std::vector<std::string> _codes;
    };


}

