#include <memory>
#include "tlv/tlv.h"

class TableCursor
{
    public:
    TableCursor();
    ~TableCursor();

    int next();

};
class TableOperator
{
    public:
    TableOperatorer();
    ~TableOperatorer();

    int insert_record();
    int delete_record();
    int scan_record();
    int get_record();
};

typedef std::shared_ptr<TableOperator> TableOperatorPtr;