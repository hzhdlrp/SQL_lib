#include <iostream>
#include "lib/memdb.hpp"
#include "lib/parser.hpp"

int main() {

    std::regex r(R"((?:\{[^}]+\}\s*)?\w+\s*:\s*\w+(?:\[\d+\])?(?:\s*=\s*[^,)]+)?,?)");
    std::string s(" ({key, autoinc} id : int32 = 5 + 10 + |string|, {unique} login : string[32] = \"abcde\" + \"abc\", is_admin : bool = false)");
    std::cout << s << "\n\n";
    std::vector<std::string> columns_info;
    for (std::smatch sm; regex_search(s, sm, r);)
    {
        columns_info.push_back(sm.str());
        s= sm.suffix();
    }

    for (auto &column_info : columns_info) {
        std::cout << "column attributes: \n";
        std::vector<std::string> attributes;
        std::regex r1(R"((?:\{[^}]+\}\s*)?)");
        std::string attr;
        std::smatch smatch;
        std::regex_search(column_info, smatch, r1);
        attr = smatch.str();

        column_info = smatch.suffix();

        std::cout << attr << '\n';

        for (std::smatch sm; regex_search(attr, sm, std::regex(R"([^,{}\s]+)"));)
        {
            attributes.push_back(sm.str());
            attr = sm.suffix();
        }

        for (auto &a : attributes) {
            std::cout << a << '\n';
        }


        std::string columnName;
        std::regex_search(column_info, smatch, std::regex(R"([^\s:]+)"));

        columnName = smatch.str();
        column_info = smatch.suffix();

        std::cout << "column name = " << columnName << "\n";

        std::string columnType;
        std::regex_search(column_info, smatch, std::regex(R"([^\s:=]+)"));

        columnType = smatch.str();
        column_info = smatch.suffix();

        std::cout << "column type = " << columnType << '\n';

        std::cout << column_info << "\n";
        std::cout << "\n";
    }

    return 0;
}