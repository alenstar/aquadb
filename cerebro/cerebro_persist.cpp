#include "cerebro_persist.h"
#include "util/logdef.h"

int create_tick_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname)
{
    // build table metadata
    auto tbl = std::make_shared<aquadb::TableDescriptor>();
    tbl->name = tblname;
    std::vector<uint16_t> keyids;

    // build key field
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "dt";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int32;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "ts";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int64;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "symbol";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::FixedString;
    kfield.len = 16; // max key size
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }


    // build val field
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "last";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "high";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "low";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "open";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "prev_close";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "turnover";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "volume";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "open_interest";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "prev_open_interest";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "settlement";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "prev_settlement";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "limit_down";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "limit_up";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }

    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "ask_vols";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::FixedString;
    vfield.len = sizeof(double) * 5;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "asks";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::FixedString;
    vfield.len = sizeof(double) * 5;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "bid_vols";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::FixedString;
    vfield.len = sizeof(double) * 5;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "bids";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::FixedString;
    vfield.len = sizeof(double) * 5;
    tbl->add_field(vfield);
    }


    // set primary key
    tbl->set_primary_key(keyids);

    int rc = mgr->create_table(dbname, tbl);
    if(rc != 0)
    {
        LOGE("create table failed: rc=%d, dbname=%s tblname=%s", rc, dbname.c_str(), tblname.c_str());
    }
    return rc;
}


int create_account_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname)
{
        // build table metadata
    auto tbl = std::make_shared<aquadb::TableDescriptor>();
    tbl->name = tblname;
    std::vector<uint16_t> keyids;

    // build key field
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "dt";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int32;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "aid";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int64;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }

    // build val field
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "margin";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "buy_margin";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "sell_margin";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "cash";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "daily_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "position_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "market_value";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "equity";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "frozen_cash";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "trading_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "transaction_cost";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "units";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "unit_value";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }

    // set primary key
    tbl->set_primary_key(keyids);

    int rc = mgr->create_table(dbname, tbl);
    if(rc != 0)
    {
        LOGE("create table failed: rc=%d, dbname=%s tblname=%s", rc, dbname.c_str(), tblname.c_str());
    }
    return rc;
}

int create_order_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname)
{
    // build table metadata
    auto tbl = std::make_shared<aquadb::TableDescriptor>();
    tbl->name = tblname;
    std::vector<uint16_t> keyids;

    // build key field
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "trading_date";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int32;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "aid";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int64;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "symbol";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::FixedString;
    kfield.len = 16; // max key size
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }

    // build val field
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "margin";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "buy_margin";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "sell_margin";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "cash";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "daily_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "position_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "market_value";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "equity";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "frozen_cash";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "trading_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "transaction_cost";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "units";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "unit_value";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }

    // set primary key
    tbl->set_primary_key(keyids);

    int rc = mgr->create_table(dbname, tbl);
    if(rc != 0)
    {
        LOGE("create table failed: rc=%d, dbname=%s tblname=%s", rc, dbname.c_str(), tblname.c_str());
    }
    return rc;
}

int create_position_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname)
{
    // build table metadata
    auto tbl = std::make_shared<aquadb::TableDescriptor>();
    tbl->name = tblname;
    std::vector<uint16_t> keyids;

    // build key field
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "dt";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int32;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "symbol";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::FixedString;
    kfield.len = 16; // max key size
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }
    {
    aquadb::FieldDescriptor kfield;
    kfield.name = "direction";
    kfield.id = tbl->next_id();
    kfield.flags = static_cast<uint16_t>(aquadb::FieldDescriptor::FieldFlag::PrimaryKey);
    kfield.type = aquadb::FieldDescriptor::FieldType::Int8;
    keyids.push_back(kfield.id);
    tbl->add_field(kfield);
    }

    // build val field
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "closable";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "margin";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "market_value";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "position_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "quantity";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "today_closable";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "trading_pnl";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "avg_price";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }
    {
    aquadb::FieldDescriptor vfield;
    vfield.name = "commissions";
    vfield.id = tbl->next_id();
    vfield.type = aquadb::FieldDescriptor::FieldType::Float64;
    tbl->add_field(vfield);
    }

    // set primary key
    tbl->set_primary_key(keyids);

    int rc = mgr->create_table(dbname, tbl);
    if(rc != 0)
    {
        LOGE("create table failed: rc=%d, dbname=%s tblname=%s", rc, dbname.c_str(), tblname.c_str());
    }
    return rc;
}

int aquadb_record_to_tick(const aquadb::TupleObject& record, CerebroTickRecord& tick)
{
    // TODO
    return -1;
}

int tick_to_aquadb_record(const  CerebroTickRecord& tick,aquadb::TupleObject& record)
{
    return 0;
}

