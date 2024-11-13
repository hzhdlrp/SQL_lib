#include "memdb.hpp"
#include <gtest/gtest.h>

using namespace memdb;

TEST(memdb, constructors1) {
    Column<int32_t> c1("column1", 0, 0);
    Column<std::string> c2("column2", 0);

    Table table("table1", std::vector{c1}, std::vector{c2}, std::vector<Column<bool>>{}, std::vector<Column<std::vector<uint8_t>>>{});

    ASSERT_EQ(table.getColumnsCount(), 2);
    ASSERT_EQ(table.name, "table1");
    ASSERT_EQ(c1.name, "column1");
}

TEST(memdb, constructor2) {
    Column<int32_t> c1("column1", 0, 0);
    Column<std::string> c2("column2", 0);

    Table table("table1", std::vector{c1}, std::vector{c2}, std::vector<Column<bool>>{}, std::vector<Column<std::vector<uint8_t>>>{});

    }

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



