#include <cstring>
#include <fstream>
#include <thread>
#include <chrono>
#include <memory>
#include <iomanip>
#include <set>
#include <mutex>
#include <unordered_map>
#include <set>
#include "util/logdef.h"
//#include "util/common.h
#include "TencentApi.h"


using namespace std;
using namespace tencent_api;

int main(int argc, char* argv[])
{
	string url_test = "http://qt.gtimg.cn/q=";
    tencent_api::TencentApi api;
	api.set_url(url_test);

    vector<shared_ptr<tencent_api::MarketQuote>> quotes;

    vector<string> codes;
    codes.push_back("sz000001");
    codes.push_back("sh000001");
    codes.push_back("sh502028");
    codes.push_back("sz399001");
    api.get_quotes(codes, &quotes);

	cout << "test tencent\n";
    for (auto q : quotes) {
        cout << q->code << "," << q->date << "," << q->time << "," << q->last << "," << q->volume << endl;
    }
    getchar();
}


