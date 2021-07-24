#pragma once
#include <memory>
#include "descriptor.h"
// #include "rocksdb/db.h"

namespace rocksdb{
    class Iterator;
}

namespace aquadb
{

#define MY_DISABLE_COPY(CLASS_NAME)  CLASS_NAME(const CLASS_NAME&) = delete; CLASS_NAME& operator=(const CLASS_NAME&) = delete

class TableWriter
{
    public:
    TableWriter(DatabaseDescriptorPtr db, TableDescriptorPtr tbl):_db(db),_tbl(tbl) {}
    ~TableWriter() = default;

    int insert(const std::vector<Value>& row);
    int insert(const TupleRecord& record);
    int insert(const std::vector<TupleRecord>& records);

    //int replace(const TableRecord& record);
    //int replace(const std::vector<TableRecord>& records);

    int remove(const std::vector<Value>& row);
    //int remove(const TupleRecord& record);
    //int remove(const std::vector<TableRecord>& records);
    int remove_range(const std::vector<Value>& start_key, const std::vector<Value>& end_key);

private:
    MY_DISABLE_COPY(TableWriter);

    private:
    DatabaseDescriptorPtr _db;
    TableDescriptorPtr _tbl;
    std::string _errmsg;
};
typedef std::shared_ptr<TableWriter> TableWriterPtr;

class TableReader
{
    public:
    TableReader(DatabaseDescriptorPtr db, TableDescriptorPtr tbl):_db(db),_tbl(tbl) {}
    ~TableReader() = default;

    int seek_to_first(const IndexDescriptor* index, const std::vector<Value>& key);
    int seek_to_first() {
         std::vector<Value> key;
        return seek_to_first(_tbl->get_primary_key(), key);
    }

    int seek_to_last(const IndexDescriptor* index, const std::vector<Value>& key);
    int seek_to_last()
    {
        std::vector<Value> key;
        return seek_to_first(_tbl->get_primary_key(), key);
    }

    int seek_to(const IndexDescriptor* index,  const std::vector<Value>& key,bool prefix = false);

    int next();
    int prev();

    inline const std::vector<std::string>& get_columns() const { return _names;}
    inline const std::vector<Value>& get_current_row() const { return _values;}
    inline const std::string& get_errmsg() const { return _errmsg;}
private:
    MY_DISABLE_COPY(TableReader);

    private:
    DatabaseDescriptorPtr _db;
    TableDescriptorPtr _tbl;
    std::unique_ptr<rocksdb::Iterator> _iter{nullptr};
    MutTableKey _mutkey;
    std::vector<std::string> _names;
    std::vector<Value> _values;
    std::string _errmsg;
};
typedef std::shared_ptr<TableReader> TableReaderPtr;

}