#pragma once
#include "cerebro_struct.h"
#include "db/db.h"

int create_tick_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname);
int create_account_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname);
int create_order_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname);
int create_position_table(aquadb::DBManager* mgr, const std::string& dbname, const std::string& tblname);

int aquadb_record_to_tick(const aquadb::TupleRecord& record, CerebroTickRecord& tick);
int tick_to_aquadb_record(const  CerebroTickRecord& tick, aquadb::TupleRecord& record);