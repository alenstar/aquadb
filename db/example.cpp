#include <iostream>
#include "db.h"

int main(int argc ,char* argv[])
{
    DBManager dbm;
    dbm.init("db_dir");
    dbm.open("db_test", false);
    return 0;
}