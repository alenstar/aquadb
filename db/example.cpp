#include <iostream>
#include "db.h"

using namespace aquadb;
int main(int argc ,char* argv[])
{
    DBManager* dbm = DBManager::get_instance();
    dbm->init("db_dir");
    dbm->open("db_test", false);
    return 0;
}