#include "memdb.hpp"
#include "parser.hpp"

memdb::ColumnInterface::ColumnInterface(std::string &s) {name = s;}
memdb::ColumnInterface::ColumnInterface(std::string &s, ColumnAttributes &a) {name = s; attributes = a;}

template <typename T>
memdb::Column<T>::Column(std::string &&s) : ColumnInterface(s) {}

template<typename T>
memdb::Column<T>::Column(std::string &&s, ColumnAttributes &&a) : ColumnInterface(s, a) {}

template <typename T>
memdb::Column<T>::Column(std::string &&s, T &&v) :  ColumnInterface(s) {value = v;}

template<typename T>
memdb::Column<T>::Column(std::string &&s, ColumnAttributes &&a, T &&v)  : ColumnInterface(s, a) {value = v;}

template<typename... Types>
void memdb::Table<Types...>::insert(Line<Types...> &&l) {lines.push_back(l);}

template<typename... Types>
std::type_info memdb::Table<Types...>::getType() const {
    return typeid(Table<Types...>);
}
