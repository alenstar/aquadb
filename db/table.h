#pragma once
#include <memory>
#include "table_info.h"

class TableCursor
{
    public:
    TableCursor();
    ~TableCursor();

    int next();

};

class TableIndex
{
    public:
    TableIndex();
};

class TableRecord
{
    public:
    TableRecord();
    TableIndex get_pk_index() const; // 主键索引
    TableIndex get_uq_index() const; // 唯一索引
    std::string get_data() const;
};

class TableWriter
{
    public:
    TableWriter();
    ~TableWriter();
    int insert(const TableRecord& record);
    int insert(const std::vector<TableRecord>& records);

    int replace(const TableRecord& record);
    int replace(const std::vector<TableRecord>& records);

    int remove(const TableRecord& record);
    int remove(const std::vector<TableRecord>& records);
    int remove(const TableRecord& start_record, const TableRecord& end_record);

};
class TableReader
{
    public:
    TableReader();
    ~TableReader();

    int seek_to_first(const TableIndex& index);
    int seek_to_last(const TableIndex& index);
    int seek_to(const TableIndex& index, bool prefix = false);

    int next(TableRecord& record);
    int prev(TableRecord& record);

    private:
    std::string name;
};
