#include "memdb.hpp"
#include "parser.hpp"
#include <gtest/gtest.h>

using namespace memdb;

TEST(createTable, tablesNames) {

    Database db;
    Parser p;
    try {
        execute(db, p.parse("create table users ({key, autoincrement} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    } catch(const std::exception &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }    ASSERT_EQ(db.getTablesNamesString(), "users|");
    // | <- разделитель имен таблиц в выводе getTablesNamesString()
}

TEST(createTable, columnsNames) {

    Database db;
    Parser p;
    try {
        execute(db, p.parse("create table users ({key, autoincrement} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    } catch(const std::exception &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }    ASSERT_EQ(db.getTablesColumnsString(), "id|login|is_admin|");
}

TEST(createTable, columnsCount) {
    Database db;
    Parser p;
    try {
        execute(db, p.parse("create table users ({key, autoincrement} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    } catch(const std::exception &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }    ASSERT_EQ(db.getTablesNamesString(), "users|");
}

TEST(insert, def) {



    // | <- разделитель имен таблиц в выводе getTablesNamesString()
}

int main(int argc, char **argv) {
//    testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
    Database db;
    Parser p;
    try {
        execute(db, p.parse("create table users ({key, autoincrement} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        execute(db, p.parse("insert (login = \"agent007\", is_admin = true) to users"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        db.uploadToFile("/Users/vadimleonov/Desktop/плюсы/SQL_lib/lib/file.txt");
    } catch (const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }


}



