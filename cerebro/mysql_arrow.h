#include <mysql/mysql.h>

std::shared_ptr<arrow::RecordBatch> convertArrow(MYSQL_RES *, int);
