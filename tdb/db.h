#include <iostream>
#include "lmdb/lmdb++.h"
#include "table.h"
#include "tlv/tlv.h"

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
    int open(const DBOption& opt);
    int open(const std::string& dbname, bool readonly);
    int close(const std::string& dbname);
    int close_all();

    TableOperatorPtr get_table_operator(const std::string& dbname, const std::string& tblname);
    private:
    std::string _basepath;
    MDB_env *_env{nullptr};
    MDB_dbi _dbi;
};