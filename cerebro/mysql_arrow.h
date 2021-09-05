#include <mysql/mysql.h>
#include <arrow/record_batch.h>
#include <memory>

std::shared_ptr<arrow::RecordBatch> convertArrow(MYSQL_RES *, int);
