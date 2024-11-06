#include "memdb.hpp"
#include "parser.hpp"



memdb::ColumnInterface::ColumnInterface(std::string &s) {name = s;}
memdb::ColumnInterface::ColumnInterface(std::string &s, ColumnAttributes &a) {name = s; attributes = a;}

template <typename T>
memdb::Column<T>::Column(std::string &&s, int lenght) : ColumnInterface(s) {len = lenght;}

template<typename T>
memdb::Column<T>::Column(std::string &&s, ColumnAttributes && a, int lenght) : ColumnInterface(s, a) {len = lenght;}

template <typename T>
memdb::Column<T>::Column(std::string &&s, T &&v, int lenght) :  ColumnInterface(s) {value = v; hasDefoltValue = true; len=lenght;}

template<typename T>
memdb::Column<T>::Column(std::string &&s, ColumnAttributes &&a, T &&v, int lenght)  : ColumnInterface(s, a) {value = v; hasDefoltValue = true; len = lenght;}

template<typename... Types>
memdb::Table::Table(std::string &&s, std::tuple<Column<Types>...> &&columns) {
    name = s;
    for (auto column : columns) {

        if (typeid(column.value) == typeid(int)) {
            intColumns.push_back(column);
        } else if (typeid(column.value) == typeid(std::string)) {
            stringColumns.push_back(column);
        } else if (typeid(column.value) == typeid(bool)) {
            boolColumns.push_back(column);
        } else if (typeid(column.value) == typeid(std::vector<uint8_t>)) {
            byteColumns.push_back(column);
        }

        columnsCount++;
    }
}

template<int ArgumentsCount, typename... Types>
memdb::Line<ArgumentsCount, Types...>::Line(bool b, std::tuple<std::pair<std::string, Types>...> &t) {
    hasNames = b;
    values = t;
}

template<int ArgumentsCount, typename... Types>
void memdb::Table::insert(Line<ArgumentsCount, Types...> &&line) {
    if (line.count != columnsCount) {
        throw ExecutionException("an attempt to insert an inappropriate number of values");
    }

}