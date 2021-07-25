#include <cstring>
#include <fstream>
#include <thread>
#include <chrono>
#include <memory>
#include <iomanip>
#include <set>
#include "util/logdef.h"
#include "util/common.h"
#include <unistd.h>
#include <getopt.h>
#include <mutex>
#include <unordered_map>
#include <set>
#include "SinaApi.h"


using namespace std;
using namespace sina_api;

int main(int argc, char* argv[])
{
    sina_api::SinaApi api;
    api.set_url("http://hq.sinajs.cn/list=");

    vector<shared_ptr<sina_api::MarketQuote>> quotes;


    vector<string> codes;
    codes.push_back("sz000001");
    codes.push_back("sh000001");
    codes.push_back("sh502028");
    codes.push_back("sz399001");
    api.get_quotes(codes, &quotes);

    for (auto q : quotes) {
        cout << q->code << "," << q->date << "," << q->time << "," << q->last << "," << q->volume << endl;
    }
    getchar();
    return 0;
}

