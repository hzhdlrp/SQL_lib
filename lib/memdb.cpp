#include "memdb.hpp"

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