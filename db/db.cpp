#include <iostream>
#include "util/fileutil.h"
#include "util/logdef.h"
#include "db.h"

namespace aquadb
{

DBManager::~DBManager()
{
}

int DBManager::init(const std::string &basepath)
{
    _basepath = basepath;
    auto ptr = RocksWrapper::get_instance();
    int rc=  ptr->init(basepath);
    if(rc != 0)
    {
        LOGE("rocksdb wrapper init failed: %s", basepath.c_str());
        return rc;
    }

    MutTableKey key;
    key.append_u32(kMetaDatabaseId);
    rocksdb::Slice k(key.data(), key.size());
    auto cursor = ptr->seek_for_next(ptr->get_meta_info_handle(), k, true);
    for (; cursor->Valid(); cursor->Next())
    {
        if (!cursor->key().starts_with(k))
        {
            break;
        }
        // TODO
        auto dbdesc = std::make_shared<DatabaseDescriptor>();
        auto v = cursor->value();
        BufferView buf(v.data(), v.size());
        dbdesc->deserialize(buf);

        // TODO
        // lock scope
        _name2dbi[dbdesc->name] = dbdesc;
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
        return ERR_DB_NOT_FOUND;
    }
    
    auto ptr = RocksWrapper::get_instance();
    MutTableKey key;
    key.append_u32(kMetaTableId);
    key.append_u32(it->second->id);
    rocksdb::Slice k(key.data(), key.size());
    auto cursor = ptr->seek_for_next(ptr->get_meta_info_handle(), k, true);
    for (; cursor->Valid(); cursor->Next())
    {
        if (!cursor->key().starts_with(k))
        {
            break;
        }
        // TODO
        TableDescriptorPtr tbdesc = std::make_shared<TableDescriptor>();
        auto v = cursor->value();
        BufferView buf(v.data(), v.size());
        tbdesc->deserialize(buf);
        it->second->add_table(tbdesc->name, tbdesc);
    }
    return 0;
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

    auto dbptr = std::make_shared<DatabaseDescriptor>();
    dbptr->name = dbname; 
    dbptr->id = last_dbid;
    auto k = dbptr->get_key(kMetaDatabaseId);
    auto v = dbptr->get_val();
    rocksdb::Slice key(k.data(), k.size());
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
    else 
    {
        // TODO
        // lock scope
        _name2dbi[dbname] = dbptr;
    }
    return status.code();
}

int DBManager::create_kv_table(const std::string& dbname, const std::string& tblname)
{
    auto db = get_db_descriptor(dbname);
    if(!db)
    {
        return ERR_DB_NOT_FOUND;
    }
    
    if(db->exists(tblname))
    {
        // TODO
        return 0;
    }

    // build table metadata
    auto tbl = std::make_shared<TableDescriptor>();
    tbl->name = tblname;
    tbl->id = db->next_id(); 
    // build key field
    FieldDescriptor kfield;
    kfield.name = "k";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = FieldDescriptor::FieldType::FixedString;
    kfield.len = 120; // max key size
    // build val field
    FieldDescriptor vfield;
    vfield.name = "v";
    vfield.id = tbl->next_id();
    vfield.type = FieldDescriptor::FieldType::String;

    tbl->add_field(kfield);
    tbl->add_field(vfield);
    tbl->set_primary_key({kfield.id});


    // 
    auto k = tbl->get_key(kMetaTableId, db->id);
    auto v = tbl->get_val();
    rocksdb::Slice key(k.data(), k.size());
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
        return static_cast<int>(status.code());
    }
    else 
    {
        // TODO
        // lock scope
        db->add_table(tbl->name, tbl);
    }

return 0;
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


 TableReaderPtr DBManager::get_table_reader(const std::string &dbname, const std::string &tblname){
     auto db = get_db_descriptor(dbname);
     if(!db)
     {
         return nullptr;
     }
     auto tbl = db->get_table(tblname);
     if(!tbl)
     {
         return nullptr;
     }
    auto reader = std::make_shared<TableReader>(db, tbl);
    return reader;
 }
 TableWriterPtr DBManager::get_table_writer(const std::string &dbname, const std::string &tblname){
     auto db = get_db_descriptor(dbname);
     if(!db)
     {
         return nullptr;
     }
     auto tbl = db->get_table(tblname);
     if(!tbl)
     {
         return nullptr;
     }
    auto writer = std::make_shared<TableWriter>(db, tbl);
    return writer;
 }

TableDescriptorPtr DBManager::get_table_descriptor(const std::string &dbname, const std::string &tblname)
{
     auto db = get_db_descriptor(dbname);
     if(!db)
     {
         return nullptr;
     }
     auto tbl = db->get_table(tblname);
     if(!tbl)
     {
         return nullptr;
     }
     return tbl;
}

}