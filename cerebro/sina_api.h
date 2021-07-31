#pragma once


#include <memory>
#include <string>
#include <vector>
#include "mdlink.h"
#include "httplib/httplib.h"

namespace sina_api {

    class SinaApi: public mdlink_api::MdLinkApi {
    public:

        SinaApi(int page_size =15);

        virtual ~SinaApi();
        int get_quotes_by_url(const std::string& url,std::function<bool(mdlink_api::MarketQuotePtr)> fn);
        int get_quotes(const std::vector<std::string>& codes,std::function<bool(mdlink_api::MarketQuotePtr)> fn) override;

        static std::string convert_to_inner_code(const std::string& code);
        static std::string convert_to_extend_code(const std::string& code);
        //static std::string convert_to_extend_code(const char* code) {
        //    return convert_to_extend_code(std::string(code));
        //}
    private:
        size_t _page_size {15};
         httplib::Client* _cli {nullptr};
    };


}

