#pragma once

#include <iostream>
#include "rocks_wrapper.h"
#include "table.h"
#include "descriptor.h"

namespace aquadb
{
struct DBOption
{
    std::string dbname;
    std::string dbpath;
    bool readonly;
};


class DBManager
{
    public:
    static DBManager* get_instance() {
        static DBManager _instance;
        return &_instance;
    }
    DBManager() = default;
    ~DBManager();

    int init(const std::string& basepath);
    int init(const char* basepath)
    {
        std::string s(basepath);
        return init(s);
    }
    int open(const std::string& dbname, bool readonly = true);
    int open(const char* dbname, bool readonly = true)
    {
        std::string s(dbname);
        return open(s, readonly);
    }
    int create(const std::string& dbname);
    int close(const std::string& dbname);
    int close(const char* dbname)
    {
        std::string s(dbname);
        return close(s);
    }
    int close_all();
    bool exists(const std::string& dbname);
    bool drop(const std::string& dbname);

    TableReader get_table_reader(const std::string& dbname, const std::string& tblname);
    TableWriter get_table_writer(const std::string& dbname, const std::string& tblname);
    private:
     DBManager();
    std::string _basepath;
    //lmdb::env _env{nullptr};
    //std::map<std::string,uint32_t> _name2dbi;
    std::map<std::string,DatabaseDescriptor*> _name2dbi;
    uint32_t _last_dbid{0}; // 最近的库id
};

}