#include "memdb.hpp"
#include "parsing/parser.hpp"
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
    // | <- разделитель имен таблиц в выводе
}

TEST(createTable, columnsNames) {

    Database db;
    Parser p;
    try {
        execute(db, p.parse("create table users ({key, autoincrement} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    } catch(const std::exception &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }    ASSERT_EQ(db.getTablesColumnsString(), "id|login|is_admin|");
    // | <- разделитель имен столбцов в выводе
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

TEST(insert, upload) {
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
        execute(db, p.parse("insert (, login = \"mylogin\") to users"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        db.uploadToFile("../../lib/test_uploads/file.txt");
    } catch (const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
}

TEST(select, upload) {
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
        execute(db, p.parse("insert (, login = \"mylogin\") to users"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }

    try {
        std::get<Table>(execute(db, p.parse("select id, login from users where is_admin || id < 10"))).uploadToFile("../../lib/test_uploads/file1.txt");
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }

}

TEST(select, multiline) {
    Database db;
    Parser p;
    try {
        execute(db, p.parse("create table "
                            "users ({key, autoincrement} id : int32 = 5 , {unique}"
                            " login : string[32] =\"abcde\", is_admin : bool =  false  )"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        execute(db, p.parse(
                "insert (login = \"agent007\", "
                        "is_admin = true) to users"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        execute(db, p.parse(
                "insert (,"
                        " login = \"mylogin\") to "
                        "users"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }

    try {
        std::get<Table>(execute(db, p.parse(
                "select id, login "
                        "from users where is_admin ||"
                        " id < 10"
                        ))).uploadToFile("../../lib/test_uploads/file2.txt");
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }

}

TEST(del, upload) {
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
        execute(db, p.parse("insert (, login = \"mylogin\") to users"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        execute(db, p.parse("delete users where login == \"agent007\""));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        db.uploadToFile("../../lib/test_uploads/file3.txt");
    } catch (const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
}

TEST(update, upload) {
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
        execute(db, p.parse("insert (, login = \"mylogin\") to users"));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        execute(db, p.parse("delete users where login == \"agent007\""));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        execute(db, p.parse("update users set is_admin = true where login == \"mylogin\""));
    } catch(const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
    try {
        db.uploadToFile("../../lib/test_uploads/file4.txt");
    } catch (const ExecutionException &e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }
}

// в последних тестах происходит выгрузка бд или временных таблиц в файлы

// ЗАПРЕЩЕНО
//  использовать в строковых значениях символы ,()
// да и в целом писать плохие строки и запросы

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

