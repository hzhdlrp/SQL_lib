#include "memdb.hpp"
#include "parser.hpp"
#include <gtest/gtest.h>

using namespace memdb;

TEST(createTable, tablesNames) {

    Database db;
    Parser p;
    execute(db, p.parse("create table users ({key, autoinc} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    ASSERT_EQ(db.getTablesNamesString(), "users|");
    // | <- разделитель имен таблиц в выводе getTablesNamesString()
}

TEST(createTable, columnsNames) {

    Database db;
    Parser p;
    execute(db, p.parse("create table users ({key, autoinc} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    ASSERT_EQ(db.getTablesColumnsString(), "id|login|is_admin|");
}

TEST(createTable, columnsCount) {
    Database db;
    Parser p;
    execute(db, p.parse("create table users ({key, autoinc} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    ASSERT_EQ(db.getTablesNamesString(), "users|");
}

int main(int argc, char **argv) {
//    testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
    Database db;
    Parser p;
    execute(db, p.parse("create table users ({key, autoinc} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    std::cout << db.getTablesColumnsString() << '\n' << db.getTablesNamesString();

}



