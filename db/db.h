#pragma once
#include <iostream>
//#include "lmdb/lmdb++.h"
#include "lmdb/lmdb.h"
#include <db.h>
#include "table.h"
#include "table_info.h"

struct DBOption
{
    std::string dbname;
    std::string dbpath;
    bool readonly;
};

class DBManager
{
    public:
    DBManager();
    ~DBManager();
    int init(const std::string& basepath);
    int init(const char* basepath)
    {
        std::string s(basepath);
        return init(s);
    }
    int open(const DBOption& opt);
    int open(const std::string& dbname, bool readonly = true);
    int open(const char* dbname, bool readonly = true)
    {
        std::string s(dbname);
        return open(s, readonly);
    }
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
    std::string _basepath;
    MDB_env *_env{nullptr};
    //lmdb::env _env{nullptr};
    MDB_dbi* _dbi{nullptr};
    std::map<std::string,MDB_dbi> _name2dbi;
};