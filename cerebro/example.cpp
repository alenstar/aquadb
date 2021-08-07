#include <iostream>
#include <chrono>
#include "cerebro_struct.h"
#include "cerebro_broker.h"
#include "sina_api.h"
#include "tencent_api.h"

int main(int argc ,char* argv[])
{
    int rc =0 ;
    std::vector<std::string> codes;//{"sz000001","sh600001","sh688681","sh688680"};
    mdlink_api::MarketDataProvider mdlink;
    mdlink.init(8);
    rc = mdlink.update_codes();
    if(rc != 0)
    {
        std::cerr << "get_codes failed" << std::endl;
    }
    std::cout << "codes.size=" << mdlink.get_codes().size() << std::endl;
    mdlink.update_quotes();
    mdlink.wait_for_shutdown();
    /*
    // TODO
    auto sina = sina_api::SinaApi();
    sina.get_quotes(codes,[](mdlink_api::MarketQuotePtr q){
         //std::cout << q->code << "," << q->date << "," << q->time << "," << q->last << "," << q->volume << std::endl;
         std::cout << *q << std::endl;
        return true;
    });

    auto tencent = tencent_api::TencentApi();
    tencent.get_quotes(codes,[](mdlink_api::MarketQuotePtr q){
         //std::cout << q->code << "," << q->date << "," << q->time << "," << q->last << "," << q->volume << std::endl;
         std::cout << *q << std::endl;
        return true;
    });
    */

    //std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}
