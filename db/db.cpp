#include <iostream>
#include <db.h>
#include "util/fileutil.h"
#include "util/logdef.h"
#include "db.h"
#include "my_rocksdb.h"

namespace aquadb
{

DBManager::~DBManager()
{
    for(auto i : _name2dbi)
    {
        delete i.second;
    }
}

int DBManager::init(const std::string &basepath)
{
    _basepath = basepath;
    auto ptr = RocksWrapper::get_instance();
    int rc=  ptr->init(basepath);
    if(rc != 0)
    {
        return rc;
    }

    MutTableKey key;
    key.append_u32(kMetaDatabaseId);
    rocksdb::Slice k(key.data().data(), key.size());
    auto cursor = ptr->seek_for_next(ptr->get_meta_info_handle(), k, true);
    for (; cursor->Valid(); cursor->Next())
    {
        if (!cursor->key().starts_with(k))
        {
            break;
        }
        // TODO
        DatabaseDescriptor dbdesc;
        auto v = cursor->value();
        BufferView buf(v.data(), v.size());
        dbdesc.deserialize(buf);
    }
    return rc;
}


int DBManager::open(const std::string &dbname, bool readonly)
{
    auto it = _name2dbi.find(dbname);
    if(it == _name2dbi.cend())
    {
        // TODO
        // not found db
        return -1;
    }
    
    auto ptr = RocksWrapper::get_instance();
    MutTableKey key;
    key.append_u32(kMetaTableId);
    key.append_u32(it->second->id);
    rocksdb::Slice k(key.data().data(), key.size());
    auto cursor = ptr->seek_for_next(ptr->get_meta_info_handle(), k, true);
    for (; cursor->Valid(); cursor->Next())
    {
        if (!cursor->key().starts_with(k))
        {
            break;
        }
        // TODO
        TableDescriptor tbdesc;
        auto v = cursor->value();
        BufferView buf(v.data(), v.size());
        tbdesc.deserialize(buf);
    }
    return -1;
}
int DBManager::create(const std::string& dbname)
{
    uint32_t last_dbid = 0;
    {
    auto it = _name2dbi.find(dbname);
    if(it != _name2dbi.cend())
    {
        return 0;
    }
    last_dbid = _last_dbid + 1;
    _last_dbid = last_dbid;
    }

    auto dbptr = new DatabaseDescriptor();
    dbptr->name = dbname; 
    dbptr->id = last_dbid;
    auto k = dbptr->get_key(kMetaDatabaseId);
    auto v = dbptr->get_val();
    rocksdb::Slice key(k.data().data(), k.size());
    rocksdb::Slice val(reinterpret_cast<const char*>(v.data()), v.size());

    auto ptr = RocksWrapper::get_instance();
    auto handle = ptr->get_meta_info_handle();
    
    /*
    auto txndb = ptr->get_db();
    rocksdb::WriteOptions wopt;
    auto txn = txndb->BeginTransaction(wopt);
    myrocksdb::Transaction mytxn(txn);
    
    auto status = mytxn.Put(handle, key, val);
    mytxn.Commit();
    */

    rocksdb::WriteOptions wopt;
    auto status = ptr->put(wopt, handle, key, val);
    if(!status.ok())
    {
        LOGE("put error: %s", status.getState());
    }
    return status.code();
}

bool DBManager::exists(const std::string& dbname)
{
    auto it = _name2dbi.find(dbname);
    if(it == _name2dbi.cend())
    {
        return false;
    }
    return true;
}
int DBManager::close(const std::string &dbname) { return -1; }

int DBManager::close_all() { return -1; }

// TableReader DBManager::get_table_reader(const std::string &dbname, const std::string &tblname){}
// TableWriter DBManager::get_table_writer(const std::string &dbname, const std::string &tblname){}

}