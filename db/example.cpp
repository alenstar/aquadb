#include <iostream>
#include "util/logdef.h"
#include "db.h"
INITIALIZE_EASYLOGGINGPP

using namespace aquadb;
int main(int argc ,char* argv[])
{
    DBManager* dbm = DBManager::get_instance();
    int rc = dbm->init("db_dir");
    if(rc != 0)
    {
        LOGE("db init failed: %d",rc);
        return rc;
    }

    rc = dbm->create("db_test");
    if(rc != 0)
    {
        LOGE("create db failed: %d", rc);
        return rc;
    }

    rc = dbm->open("db_test", false);
    if(rc != 0)
    {
        LOGE("open db failed: %d", rc);
        return rc;
    }

    rc = dbm->create_kv_table("db_test", "kv_test");
    if(rc != 0)
    {
        LOGE("create kv table failed: %d", rc);
        return rc;
    }

    auto tbl = dbm->get_table_descriptor("db_test", "kv_test");
    if(tbl == nullptr)
    {
        LOGE("not found table descriptor: kv_test");
        return -1;
    }

    std::cout << "table descriptor:\n" << (*tbl) << std::endl;

    auto writer = dbm->get_table_writer("db_test", "kv_test");
    if(writer== nullptr)
    {
        LOGE("not found table: kv_test");
        return -1;
    }

    aquadb::Value k("hello");
    aquadb::Value v("world!");
    aquadb::TableRow row;
    row.append("k", k);
    row.append("v", v);
    rc = writer->insert(row);
    if(rc != 0)
    {
        LOGE("insert record failed: %d", rc);
    }

    auto reader = dbm->get_table_reader("db_test","kv_test");
    if(reader == nullptr)
    {
        LOGE("not found table: kv_test");
        return -1;
    }

    aquadb::Value val;
    rc = reader->get(k, val);
    if(rc != 0)
    {
        LOGE("not found value: %s, %d", k.c_str(), rc);
    }
    else 
    {
        LOGD("record: %s %s", k.c_str(), val.c_str());
    }
    return 0;
}