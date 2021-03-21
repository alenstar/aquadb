#include "util/logdef.h"
#include "db.h"

DBManager::DBManager() {}
DBManager::~DBManager() {}

int DBManager::init(const std::string &basepath) { 
    _basepath = basepath;
    return 0; }

int DBManager::open(const DBOption &opt) {
    
    std::string dbpath = _basepath + "/" + opt.dbname;
    LOGI("lmdb version:%s\n",mdb_version(0, 0, 0));
    int rc = mdb_env_create(&_env);
    if(rc){
        LOGE("mdb_env_create error,detail:%s\n", mdb_strerror(rc));
        return -1;
    }
    
    //打开数据库，如果目录为空，将在该目录内初始化一个数据库
    rc = mdb_env_open(_env,dbpath.c_str(, 0, 0644);
    if(rc){
        LOGE("mdb_env_open error,detail:%s\n", mdb_strerror(rc));
        return -1;
    }
    
    
     return -1; }

int DBManager::open(const std::string &dbname, bool readonly) { 
    DBOption opt;
    opt.dbname = dbname;
    opt.readonly = readonly;
    return return open(opt); }

int DBManager::close(const std::string &dbname) { return -1; }

int DBManager::close_all() { return -1; }

TableOperatorPtr DBManager::get_table_operator(const std::string &dbname, const std::string &tblname)
{
    return nullptr;
}