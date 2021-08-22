#include "mysql_connect.h"
#include "util/common.h"
#include "util/logdef.h"
#include <sys/time.h>
#include <ctime>


#define IS_DATETIME(t)                                                                                                 \
    ((t) == MYSQL_TYPE_TIMESTAMP || (t) == MYSQL_TYPE_DATETIME || (t) == MYSQL_TYPE_DATE || (t) == MYSQL_TYPE_TIME ||  \
     (t) == MYSQL_TYPE_NEWDATE)
#define IS_FLOAT(t)                                                                                                    \
    ((t) == MYSQL_TYPE_FLOAT || (t) == MYSQL_TYPE_DOUBLE)
#define IS_DECIMAL(t) ((t) == MYSQL_TYPE_NEWDECIMAL || (t) == MYSQL_TYPE_DECIMAL)
#define IS_INTEGER(t)                                                                                                  \
    ((t) == MYSQL_TYPE_TINY || (t) == MYSQL_TYPE_LONG || (t) == MYSQL_TYPE_SHORT || (t) == MYSQL_TYPE_INT24 ||         \
     (t) == MYSQL_TYPE_LONGLONG || (t) == MYSQL_TYPE_BIT || (t) == MYSQL_TYPE_ENUM)

using std::string;
////////////////////////////////////////////////////////////



MysqlConnect::MysqlConnect()
{
    auto p = mysql_init(&_mysql);
    if (!p)
    {
        LOGE( "mysql_init fail");
    }
}

MysqlConnect::~MysqlConnect() { mysql_close(&_mysql); }

int MysqlConnect::init(const connect_option_t& option)
{
    // enable reconnect
    my_bool reconnect = 1;
    mysql_options(&_mysql, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(&_mysql, MYSQL_OPT_READ_TIMEOUT, &option.timeout);
    mysql_options(&_mysql, MYSQL_OPT_WRITE_TIMEOUT, &option.timeout);
    mysql_options(&_mysql, MYSQL_OPT_CONNECT_TIMEOUT, &option.timeout);
    //mysql_options(&_mysql, MYSQL_READ_DEFAULT_GROUP, "");
    // disable autocommit
    my_bool autocommit = 0;
    mysql_autocommit(&_mysql, autocommit);
    // set default character set
    mysql_set_character_set(&_mysql, option.charset);

    if (!mysql_real_connect(&_mysql, option.host, option.user, option.password, option.database,
                            static_cast<uint32_t>(option.port), nullptr, 0))
    {
        LOGE( "mysql_real_connect fail: %s, %s:%d", mysql_error(&_mysql),option.host, option.port );
        return -1;
    }
    return 0;
}

int MysqlConnect::travelRecord(const string &sql,
                               const std::function<bool(const std::unordered_map<string, rowcell_t> &)> &cb, std::string *errMsg)
{
    int rc = mysql_real_query(&_mysql, sql.c_str(), sql.length());
    if (rc != 0)
    {
        if(errMsg) *errMsg = std::string(mysql_error(&_mysql));
        LOGE("mysql_real_query fail: %s, %s",  mysql_error(&_mysql) ,sql.c_str());
        return rc;
    }

    MYSQL_RES *res = mysql_store_result(&_mysql);
    if (!res)
    {
        LOGE( "mysql_store_result:not found results");
        return 0;
    }
    LOGD( "mysql_real_query: %s, %lu", sql.c_str(),  mysql_num_rows(res));
    std::unordered_map<string, rowcell_t> record;
    std::vector<std::string> names;

    uint32_t num_fields = mysql_num_fields(res);
    if (num_fields == 0)
    {
        LOGD("mysql_num_fields:not found fields");
        mysql_free_result(res);
        return 0;
    }
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    if (!fields)
    {
        LOGD("mysql_fetch_fields|not found fields");
        mysql_free_result(res);
        return 0;
    }
    for (uint32_t i = 0; i < num_fields; ++i)
    {
        names.emplace_back(std::string(fields[i].name));
    }

    char buf[128];
    MYSQL_ROW row = nullptr;

    while ((row = mysql_fetch_row(res)) != nullptr)
    {
        unsigned long *lengths = mysql_fetch_lengths(res);
        for (size_t i = 0; i < num_fields; i++)
        {
                    record[names[i]].precision = fields[i].decimals;
            if (IS_DECIMAL(fields[i].type))
            {
                if (fields[i].decimals != 0)
                {
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_DOUBLE;
                    if (lengths[i] > 0)
                    {
                        ::memcpy(buf, row[i], lengths[i]);
                        buf[lengths[i]] = '\0';
                        record[names[i]].fval  = ::strtod(buf, nullptr);
                    record[names[i]].isnull = 0;
                    }
                    else
                    {
                        record[names[i]].fval = std::numeric_limits<double>::quiet_NaN();
                    record[names[i]].isnull = 1;
                    }
                }
                else
                {
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_LONG;
                    if (lengths[i] > 0)
                    {
                        ::memcpy(buf, row[i], lengths[i]);
                        buf[lengths[i]] = '\0';
                        record[names[i]].ival = static_cast<int64_t>(::strtoll(buf, nullptr, 10));
                    record[names[i]].isnull = 0;
                    }
                    else
                    {
                    record[names[i]].isnull = 1;
                        record[names[i]].ival = static_cast<int64_t>(0);
                    }
                }
            }
            else if ( IS_FLOAT(fields[i].type))
            {
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_DOUBLE;
                    if (lengths[i] > 0)
                    {
                        ::memcpy(buf, row[i], lengths[i]);
                        buf[lengths[i]] = '\0';
                        record[names[i]].fval  = ::strtod(buf, nullptr);
                    record[names[i]].isnull = 0;
                    }
                    else
                    {
                        record[names[i]].fval = std::numeric_limits<double>::quiet_NaN();
                    record[names[i]].isnull = 1;
                    }
            }
            else if (IS_INTEGER(fields[i].type))
            {
                if(fields[i].flags & UNSIGNED_FLAG){
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_ULONG;
                }
                else {
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_LONG;
                }
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                if(fields[i].flags & UNSIGNED_FLAG){
                    record[names[i]].uval = static_cast<uint64_t>(::strtoull(buf, nullptr, 10));
                }
                else {
                    record[names[i]].ival = static_cast<int64_t>(::strtoll(buf, nullptr, 10));
                }
                    record[names[i]].isnull = 0;
                }
                else
                {
                    record[names[i]].ival = static_cast<int64_t>(0);
                    record[names[i]].isnull = 1;
                }
            }
            else if (fields[i].type == MYSQL_TYPE_DATE || fields[i].type == MYSQL_TYPE_NEWDATE)
            {
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_DATE;
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                    struct tm t;
                    auto p = ::strptime(buf, "%Y-%m-%d", &t);
                    if (p)
                    {
                        record[names[i]].ival = 
                            static_cast<int64_t>((t.tm_year + 1900) * 10000L + (t.tm_mon + 1) * 100L + t.tm_mday);
                    }
                    else
                    {
                        record[names[i]].ival = static_cast<int64_t>(0);
                    }
                    record[names[i]].isnull = 0;
                }
                else
                {
                    record[names[i]].ival = static_cast<int64_t>(0);
                    record[names[i]].isnull = 1;
                }
            }
            else if (fields[i].type == MYSQL_TYPE_TIME || fields[i].type == MYSQL_TYPE_TIME2)
            {
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                    struct tm t;
                    auto p = ::strptime(buf, "H%:M:%S", &t);
                    if (p)
                    {
                        record[names[i]].ival = static_cast<int64_t>(t.tm_hour * 10000 + t.tm_min * 100 + t.tm_sec);
                    }
                    else
                    {
                        record[names[i]].ival = static_cast<int64_t>(0);
                    }
                    record[names[i]].isnull = 0;
                }
                else
                {
                    record[names[i]].ival = static_cast<int64_t>(0);
                    record[names[i]].isnull = 1;
                }
            }
            else if (IS_DATETIME(fields[i].type))
            {
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_DATETIME;
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                    struct tm t;
                    // auto p = ::strptime(buf, "%Y-%m-%d H%:M:%S", &t);
                    auto p = ::strptime(buf, "%Y-%m-%d", &t);
                    if (p)
                    {
                        record[names[i]].ival = 
                            static_cast<int64_t>((t.tm_year + 1900) * 10000L + (t.tm_mon + 1) * 100L + t.tm_mday);
                        // record[names[i]].set(static_cast<int64_t>(t.tm_hour * 10000 + t.tm_min * 100 + t.tm_sec));
                    }
                    else
                    {
                        record[names[i]].ival = static_cast<int64_t>(0);
                    }
                    record[names[i]].isnull = 0;
                }
                else
                {
                    record[names[i]].ival = static_cast<int64_t>(0);
                    record[names[i]].isnull = 1;
                }
            }
            else
            {
                    record[names[i]].wtype = ROWCELL_WIRE_TYEP_STRING;
                if (lengths[i] > 0)
                {
                    record[names[i]].sval = row[i];
                    record[names[i]].size = lengths[i];
                    record[names[i]].isnull = 0;
                }
                else {

                    record[names[i]].isnull = 1;
                }
            }
        }
        try
        {
        if (!cb(record))
        {
            break;
        }
        } catch(std::exception& e)
        {
            LOGE( "callback exception:%s, %s", e.what() ,  sql.c_str());
            break;
        }
    }

    mysql_free_result(res);

    return rc;
}


int MysqlConnect::travelRecord(const string &sql,
                               const std::function<bool(const std::vector<std::string>&,const std::vector<rowcell_t> &)> &cb, std::string *errMsg)
{
    int rc = mysql_real_query(&_mysql, sql.c_str(), sql.length());
    if (rc != 0)
    {
        if(errMsg) *errMsg = std::string(mysql_error(&_mysql));
        LOGE("mysql_real_query fail: %s, %s",  mysql_error(&_mysql) ,sql.c_str());
        return rc;
    }

    MYSQL_RES *res = mysql_store_result(&_mysql);
    if (!res)
    {
        LOGD( "mysql_store_result:not found results");
        return 0;
    }
    LOGD("mysql_real_query:%s, %lu" , sql.c_str(),  mysql_num_rows(res));
    std::vector<rowcell_t> record;
    std::vector<std::string> names;

    uint32_t num_fields = mysql_num_fields(res);
    if (num_fields == 0)
    {
        LOGD("mysql_num_fields:not found fields");
        mysql_free_result(res);
        return 0;
    }
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    if (!fields)
    {
        LOGD( "mysql_fetch_fields:not found fields");
        mysql_free_result(res);
        return 0;
    }
    for (uint32_t i = 0; i < num_fields; ++i)
    {
        names.emplace_back(std::string(fields[i].name));
        record.emplace_back(rowcell_t());
        // LOG_DEBUG << "name=" << fields[i].name << ",type=" << fields[i].type <<"," << fields[i].decimals <<"," << fields[i].length;
    }

    char buf[128];
    MYSQL_ROW row = nullptr;

    while ((row = mysql_fetch_row(res)) != nullptr)
    {
        unsigned long *lengths = mysql_fetch_lengths(res);
        for (size_t i = 0; i < num_fields; i++)
        {
                    record[i].precision = fields[i].decimals;
            if (IS_DECIMAL(fields[i].type))
            {
                if (fields[i].decimals != 0)
                {
                    record[i].wtype = ROWCELL_WIRE_TYEP_DOUBLE;
                    if (lengths[i] > 0)
                    {
                        ::memcpy(buf, row[i], lengths[i]);
                        buf[lengths[i]] = '\0';
                        record[i].fval = ::strtod(buf, nullptr);
                    record[i].isnull = 0;
                    }
                    else
                    {
                        record[i].fval = std::numeric_limits<double>::quiet_NaN();
                    record[i].isnull = 1;
                    }
                }
                else
                {
                    record[i].wtype = ROWCELL_WIRE_TYEP_DOUBLE;
                    if (lengths[i] > 0)
                    {
                        ::memcpy(buf, row[i], lengths[i]);
                        buf[lengths[i]] = '\0';
                        record[i].ival = static_cast<int64_t>(::strtoll(buf, nullptr, 10));
                    record[i].isnull = 0;
                    }
                    else
                    {
                        record[i].ival = static_cast<int64_t>(0);
                    record[i].isnull = 1;
                    }
                }
            }
            else if (IS_FLOAT(fields[i].type))
            {
                    record[i].wtype = ROWCELL_WIRE_TYEP_DOUBLE;
                    if (lengths[i] > 0)
                    {
                        ::memcpy(buf, row[i], lengths[i]);
                        buf[lengths[i]] = '\0';
                        record[i].fval = ::strtod(buf, nullptr);
                    record[i].isnull = 0;
                    }
                    else
                    {
                        record[i].fval = std::numeric_limits<double>::quiet_NaN();
                    record[i].isnull = 1;
                    }
            }
            else if (IS_INTEGER(fields[i].type))
            {
                    record[i].wtype = ROWCELL_WIRE_TYEP_LONG;
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                    record[i].ival = static_cast<int64_t>(::strtoll(buf, nullptr, 10));
                    record[i].isnull = 0;
                }
                else
                {
                    record[i].ival = static_cast<int64_t>(0);
                    record[i].isnull = 1;
                }
            }
            else if (fields[i].type == MYSQL_TYPE_DATE || fields[i].type == MYSQL_TYPE_NEWDATE)
            {
                    record[i].wtype = ROWCELL_WIRE_TYEP_STRING;
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                    struct tm t;
                    auto p = ::strptime(buf, "%Y-%m-%d", &t);
                    if (p)
                    {
                        record[i].ival = 
                            static_cast<int64_t>((t.tm_year + 1900) * 10000L + (t.tm_mon + 1) * 100L + t.tm_mday);
                    }
                    else
                    {
                        record[i].ival = static_cast<int64_t>(0);
                    }
                    record[i].isnull = 0;
                }
                else
                {
                    record[i].ival = static_cast<int64_t>(0);
                    record[i].isnull = 1;
                }
            }
            else if (fields[i].type == MYSQL_TYPE_TIME || fields[i].type == MYSQL_TYPE_TIME2)
            {
                    record[i].wtype = ROWCELL_WIRE_TYEP_LONG;
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                    struct tm t;
                    auto p = ::strptime(buf, "H%:M:%S", &t);
                    if (p)
                    {
                        record[i].ival = static_cast<int64_t>(t.tm_hour * 10000 + t.tm_min * 100 + t.tm_sec);
                    }
                    else
                    {
                        record[i].ival = static_cast<int64_t>(0);
                    }
                    record[i].isnull = 0;
                }
                else
                {
                    record[i].ival = static_cast<int64_t>(0);
                    record[i].isnull = 1;
                }
            }
            else if (IS_DATETIME(fields[i].type))
            {
                    record[i].wtype = ROWCELL_WIRE_TYEP_DATETIME;
                if (lengths[i] > 0)
                {
                    ::memcpy(buf, row[i], lengths[i]);
                    buf[lengths[i]] = '\0';
                    struct tm t;
                    // auto p = ::strptime(buf, "%Y-%m-%d H%:M:%S", &t);
                    auto p = ::strptime(buf, "%Y-%m-%d", &t);
                    if (p)
                    {
                        record[i].ival = 
                            static_cast<int64_t>((t.tm_year + 1900) * 10000L + (t.tm_mon + 1) * 100L + t.tm_mday);
                        // static_cast<int64_t>(t.tm_hour * 10000 + t.tm_min * 100 + t.tm_sec));
                    }
                    else
                    {
                        record[i].ival = static_cast<int64_t>(0);
                    }
                    record[i].isnull = 0;
                }
                else
                {
                    record[i].ival = static_cast<int64_t>(0);
                    record[i].isnull = 1;
                }
            }
            else
            {
                    record[i].wtype = ROWCELL_WIRE_TYEP_STRING;
                if(lengths[i] > 0) {
                    record[i].sval = row[i];
                    record[i].size = lengths[i];
                    record[i].isnull = 0;
                } else {
                    record[i].isnull = 1;
                    record[i].size = 1;

                }
            }
        }
        try
        {
        if (!cb(names,record))
        {
            break;
        }
        } catch(std::exception& e)
        {
            LOGE("callback exception:%s, %s", e.what(), sql.c_str());
            break;
        }
    }

    mysql_free_result(res);

    return rc;
}

int MysqlConnect::showTables(std::vector<string> &tables, string *errMsg)
{
    const std::string& sql = "SHOW TABLES";
    auto fn = [&tables](const std::unordered_map<std::string, rowcell_t>& record) -> bool{
        for(auto const& item: record)
        {
            tables.emplace_back(item.second.sval);
        }
        return true;
    };
    int rc = travelRecord(sql,fn, errMsg);
    if(rc != 0)
    {
        LOGE("travelRecord fail:%s,%d", sql.c_str(), rc);
        return rc;
    }
    return rc;
}

int MysqlConnect::showCreateTable(const string &tblname, string &out, string *errMsg)
{
    std::string sql = std::string("SHOW CREATE TABLE ") + tblname;
    std::string createsql;
    auto fn = [&createsql](const std::unordered_map<std::string, tsb::CellValue>& record) -> bool{
        if(record.size() == 1)
        {
            createsql = record.begin()->second.to_string();
        }
        else
        {
            createsql = record.at("Create Table").sval;
        }
        return false;
    };
    int rc = travelRecord(sql,fn, errMsg);
    if(rc != 0)
    {
        LOGE("travelRecord fail:%s,%d", sql.c_str(), rc);
        return rc;
    }
    return rc;
}



//////////////////////////////////////////////////////////


///////////////////////////////////////////////////////

ConnectPool::ConnectPool() {}

ConnectPool::~ConnectPool()
{
    for (auto &item : _items)
    {
        delete item.get();
    }
}

int ConnectPool::init(const connect_option_t& option, int size, const string &engine, unsigned int timeout)
{
    for (int i = 0; i < size; ++i)
    {
        // taf::TC_Mysql* mysql = new taf::TC_Mysql;
        SQLConnect* connect = nullptr;
        if(engine == "mysql")
        {
            connect = new MysqlConnect;
        }
        else if(engine == "clickhouse")
        {
            connect = new ClickHouseConnect;
        }
        else
        {
            LOGE("unknown sql connect engine:%s, %s:%d", engine.c_str(), option.host,option.port);
            return -1;
        }
        if (connect->init(dbconn, timeout) != 0)
        {
            LOGE("sql connect init fail:%s, %s:%d", engine.c_str(), option.host,option.port);
            continue;
        }

        ConnectItem item(connect);
        std::lock_guard<std::mutex> lck(_mtx);
        _items.emplace_back(item);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////

int ConnectMgr::load_library(const string &path)
{
    return 0;
}
void ConnectMgr::unload_library()
{
}

ConnectPool *ConnectMgr::createPool(const string &name, const connect_option_t& option, const string &engine, int num)
{
    ConnectPool *pool = new ConnectPool;
    int rc = pool->init(option, num, engine, _timeout);
    if (rc != 0)
    {
        LOGE("init pool fail: %s, %s:%d",name.c_str() , option.host, option.port);
        delete pool;
        return nullptr;
    }
    std::lock_guard<std::mutex> lck(_mtx);
    _pools[name] = pool;
    return pool;
}


