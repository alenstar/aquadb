#include <mysql/mysql.h>
#include <arrow/api.h>
#include <iostream>
#include <stdio.h>

std::shared_ptr<arrow::Field> getField(const char *, int, bool);

std::shared_ptr<arrow::RecordBatch> convertArrow(MYSQL_RES *result, int batchNum) {
  int num_fields;
  MYSQL_FIELD *fields;
  MYSQL_ROW row;

  num_fields = mysql_num_fields(result);
  fields = mysql_fetch_fields(result);

  std::vector<std::shared_ptr<arrow::Field>> arrowFields;

  for (int i = 0; i < num_fields; i++) {

    auto f = getField(fields[i].name, fields[i].type, !IS_NOT_NULL(fields[i].flags));
    arrowFields.push_back(f);
  }

  auto schema = arrow::schema(arrowFields);

  std::unique_ptr<arrow::RecordBatchBuilder> builder;
  auto status = arrow::RecordBatchBuilder::Make(schema, arrow::default_memory_pool(), &builder);
  if (!status.ok()) {
    std::cout << "not build" << std::endl;
    return NULL;
  }

  int loopCnt = 0;
  while ((row = mysql_fetch_row(result))) {
      for (int i = 0; i < num_fields; ++i) {
        auto type = fields[i].type;
        arrow::Status s;
        if (IS_NUM(type)) {
          s = builder->GetFieldAs<arrow::Int8Builder>(i)->Append((int)*row[i]);
        } else if (type == MYSQL_TYPE_VAR_STRING) {
          s = builder->GetFieldAs<arrow::StringBuilder>(i)->Append(row[i]);
        } else {
          std::cout << "not type" << std::endl;
        }
        if (!s.ok()) {
          std::cout << "set failed" << std::endl;
        }
      }
      loopCnt++;
      if (loopCnt >= batchNum) {
        break;
      }
  }


  std::shared_ptr<arrow::RecordBatch> batch;
  status = builder->Flush(&batch);
  if (!status.ok()) {
    std::cout << "flush failed!!" << std::endl;
    return NULL;
  }

  return batch;
}

std::shared_ptr<arrow::Field> getField(const char * name, int type, bool nullable) {
  if (IS_NUM(type)) {
    return arrow::field(name, arrow::int8(), nullable);
  } else if (type == MYSQL_TYPE_VAR_STRING) {
    return arrow::field(name, arrow::utf8(), nullable);
  } else {
    std::cout << "undefined column(" << name << ") type is " << type << std::endl;
    return arrow::field(name, arrow::null(), nullable);
  }
}
