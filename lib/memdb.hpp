#ifndef SQL_LIB_MEMBD_H
#define SQL_LIB_MEMBD_H

#endif //SQL_LIB_MEMBD_H
#include <string>
#include <memory>
#include <vector>

namespace memdb {

    struct ColumnAttributes {
        bool unique = false;
        bool autoincrement = false;
        bool key = false;
    };

    struct ColumnInterface {
        std::string name;
        ColumnAttributes attributes;

        ColumnInterface(std::string &);
        ColumnInterface(std::string &, ColumnAttributes &);

        virtual void foo() = 0;
    };

    template <typename T>
    struct Column : public ColumnInterface {
        T value;

        Column(std::string &&);

        Column(std::string &&, T &&);

        Column(std::string &&, ColumnAttributes &&attr, T &&);

        Column(std::string &&, ColumnAttributes &&);

    };

    class Table {
        std::vector<ColumnInterface> columns;
    };


    class Database {
        std::vector<Table> tables;
    };
}