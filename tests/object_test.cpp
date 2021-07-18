#define SPDLOG_TAG "TEST"
#define DOCTEST_CONFIG_NO_TRY_CATCH_IN_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "db/tuple_record.h"
#include "util/logdef.h"

using namespace aquadb;

TEST_CASE("testing the tlv_object") {
    TupleRecord obj;
    Value a(1);
    Value b(9889878);
    Value c(0);
    Value d(-1);
    Value e(12.443);
    Value f("hello world!");
    obj.insert(1, a);
    obj.insert(2, b);
    obj.insert(3, c);
    obj.insert(4, d);
    obj.insert(5, e);
    obj.insert(6, std::move(f));
    CHECK(a.to_i32() == 1);
    CHECK(f.is_none() == true);
    CHECK(obj.get(6)->to_string() == std::string("hello world!"));
    std::vector<uint8_t> out;
    obj.serialize(out);
    LOGI("serialize size=%lu", out.size());

    TupleRecord obj2;
    obj2.deserialize(out);
    CHECK(obj.size() == obj2.size());
    CHECK(obj2.get(0) == nullptr);
}
