#include <iostream>
#include "memdb.hpp"

int main() {
    memdb::Column<int32_t> c1("column1", 0, 0);
    memdb::Column<std::string> c2("column2", "abcde", 5);




    return 0;
}
