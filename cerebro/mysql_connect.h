#pragma once

#include <mysql/mysql.h>
#include <mutex>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "util/mutex.h"


enum rowcell_wire_type_t {
    // 0 none, 1 double , 2 long, 3 string, 4 ulong, 5 date, 6 datetime
    ROWCELL_WIRE_TYEP_NONE = 0,
    ROWCELL_WIRE_TYEP_DOUBLE = 1,
    ROWCELL_WIRE_TYEP_LONG = 2,
    ROWCELL_WIRE_TYEP_STRING = 3,
    ROWCELL_WIRE_TYEP_ULONG = 4,
    ROWCELL_WIRE_TYEP_DATE = 5, // date int YYYYMMDD
    ROWCELL_WIRE_TYEP_DATETIME = 6, // ? timestamp(us)
    ROWCELL_WIRE_TYEP_TIME = 7 // time int HHMMSS 
};

typedef struct  {
    //char name[63 + 1];
    union {
        char* sval;
        int64_t ival;
        uint64_t uval;// uint64_t
        double fval;
    };
    uint32_t size = 0; 
    uint8_t wtype = 0; // rowcell_wire_type_t
    uint8_t isnull = 0;
    uint8_t precision = 0;
} rowcell_t;


typedef struct {
    char* host = nullptr;
    int port = 0;
    char* user = nullptr;;
    char* password = nullptr;
    char* database = nullptr;
    char* charset = nullptr;
    int timeout = 30; // seconds
} connect_option_t;

class SQLConnect {
    public:
    virtual ~SQLConnect() {}
    virtual int init(const connect_option_t& option) = 0;
    virtual  int travelRecord(const std::string& sql, const std::function<bool(const std::unordered_map<std::string, rowcell_t>&)>& cb, std::string* errMsg = nullptr) = 0;
    virtual int travelRecord(const std::string& sql, const std::function<bool(const std::vector<std::string>& ,const std::vector<rowcell_t>&)>& cb, std::string* errMsg = nullptr) = 0;
    virtual int showTables(std::vector<std::string>& tables, std::string* errMsg=nullptr) = 0;
    virtual int showCreateTable(const std::string& tblname, std::string& out, std::string* errMsg=nullptr) = 0;
};

class MysqlConnect: public SQLConnect
{
public:
    MysqlConnect();
    ~MysqlConnect() override;
    int init(const connect_option_t& option) override;
    int travelRecord(const std::string& sql, const std::function<bool(const std::unordered_map<std::string, rowcell_t>&)>& cb, std::string* errMsg = nullptr) override;
    int travelRecord(const std::string& sql, const std::function<bool(const std::vector<std::string>& ,const std::vector<rowcell_t>&)>& cb, std::string* errMsg = nullptr) override;
    int showTables(std::vector<std::string>& tables, std::string* errMsg=nullptr) override;
    int showCreateTable(const std::string& tblname, std::string& out, std::string* errMsg=nullptr) override;
    //int executeSQL(const std::string& sql, std::string* errMsg=nullptr);
private:
    MYSQL _mysql;
};

class ConnectPool;
class ConnectItem
{
public:
    //ConnectItem(taf::TC_Mysql* mysql):_mysql(mysql) {}
    ConnectItem(SQLConnect* mysql):_mysql(mysql) {}
    ~ConnectItem() = default;
    //taf::TC_Mysql* get() {_isused = true;return _mysql;}
    SQLConnect* get() {_isused = true;return _mysql;}
    void release() { _isused = false;}
    bool isUsed() { return _isused;}
private:
    //taf::TC_Mysql* _mysql{nullptr};
    SQLConnect* _mysql{nullptr};
    bool _isused{false};
};


class ConnectPool
{
    public:
    ConnectPool();
    ~ConnectPool();
    int init(const connect_option_t& option, int size, const std::string& engine, unsigned int timeout = 30);
    inline int getConnect(SQLConnect** mysql)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        auto it = std::find_if(_items.begin(), _items.end(), [](ConnectItem& item){ return !(item.isUsed());});
        if(it == _items.end())
        {
            return -1;
        }
        *mysql = it->get();
        return static_cast<int>(std::distance(_items.begin(), it));
    }
    inline void releaseConnect(int id)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        if(id >= static_cast<int>(_items.size()) || id < 0)
        {
            return;
        }
        _items[id].release();
    }
private:
    std::mutex _mtx;
    std::vector<ConnectItem> _items;
};

class ScopedConnectItem
{
public:
    ScopedConnectItem(int id, ConnectPool* pool):_id(id),_pool(pool) {}
    ~ScopedConnectItem() {_pool->releaseConnect(_id); _id = -1;}
    private:
    int _id{-1};
    ConnectPool* _pool{nullptr};
};

class ConnectMgr
{
public:
    ConnectMgr() {}
    ~ConnectMgr() = default;
    static int load_library(const std::string& path);
    static void unload_library();
    inline ConnectPool* getPool(const std::string& name)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        auto it = _pools.find(name);
        if(it == _pools.cend())
        {
            return nullptr;
        }
        return it->second;
    }
    int init(int num, unsigned int timeout = 15) {
        _num = num;
        _timeout = timeout;
        return 0;
    }
    ConnectPool* createPool(const std::string& name, const connect_option_t& option, const std::string& engine, int num = 4);
private:
    std::mutex _mtx;
    std::unordered_map<std::string, ConnectPool*> _pools;
    int _num {4};
    uint32_t _timeout = {15};
};
