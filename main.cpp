#include <iostream>
#include <regex>
#include <set>
#include "lib/memdb.hpp"
//#include "lib/parser.hpp"

void SortinhgStation(std::string &s) {
    std::cout << s << "\n\n";
    std::vector<std::string> operands;
    std::vector<std::string> operators;
    std::regex r;
    for (std::smatch sm; regex_search(s, sm, std::regex(R"(\+\+|--|<=|>=|==|!=|&&|\|\||\+|-|\*|/|%|>|<|\||\(|\)|!|\^)"));) {
        operators.push_back(sm.str());
        operands.push_back(sm.prefix());
        s = sm.suffix();
    }
    std::cout << "operands: \n";
    for (auto &o : operands) std::cout << o << '\n';
    std::cout << "operators: \n";
    for (auto &o : operators) std::cout << o << '\n';
}

int main() {
//    std::string s = "5 + 10 + |string|";
//    std::string s1 = "5 || 10 + |\"aaa\"| ";
//    SortinhgStation(s1);


//    std::regex r(R"((?:\{[^}]+\}\s*)?\w+\s*:\s*\w+(?:\[\d+\])?(?:\s*=\s*[^,)]+)?,?)");
//    std::string s(" ({key, autoinc} id : int32 = 5 , {unique} login : string[32] =\"abcde\", is_admin : bool =  false  )");
//    std::cout << s << "\n\n";
//    std::vector<std::string> columns_info;
//    for (std::smatch sm; regex_search(s, sm, r);)
//    {
//        columns_info.push_back(sm.str());
//        s= sm.suffix();
//    }
//
//    std::vector<std::variant<memdb::Column<int>, memdb::Column<bool>, memdb::Column<std::string>, memdb::Column<std::vector<uint8_t>>>> columnObjectVariants;
//
//    for (auto &column_info : columns_info) {
//        std::cout << "column attributes: \n";
//        std::vector<std::string> attributes;
//        std::regex r1(R"((?:\{[^}]+\}\s*)?)");
//        std::string attr;
//        std::smatch smatch;
//        std::regex_search(column_info, smatch, r1);
//        attr = smatch.str();
//
//        column_info = smatch.suffix();
//
//        std::cout << attr << '\n';
//
//        for (std::smatch sm; regex_search(attr, sm, std::regex(R"([^,{}\s]+)"));) {
//            attributes.push_back(sm.str());
//            attr = sm.suffix();
//        }
//
//        for (auto &a: attributes) {
//            std::cout << a << '\n';
//        }
//
//
//        std::string columnName;
//        std::regex_search(column_info, smatch, std::regex(R"([^\s:]+)"));
//
//        columnName = smatch.str();
//        column_info = smatch.suffix();
//
//        std::cout << "column name = " << columnName << "\n";
//
//        std::string columnType;
//        std::regex_search(column_info, smatch, std::regex(R"([^\s:=]+)"));
//
//        columnType = smatch.str();
//        column_info = smatch.suffix();
//
//        std::cout << "column type = " << columnType << '\n';
//
//        std::cout << column_info << "\n";
//
//        memdb::ColumnAttributes columnAttributes;
//        std::set<std::string> attributesSet = {"unique", "autoincrement", "key"};
//
//        for (auto &a: attributes) {
//            if (!attributesSet.contains(a)) {
////                throw ExecutionException("unknown attribute\n");
//            }
//
//            if (a == "unique") {
//                columnAttributes.unique = true;
//            } else if (a == "autoincrement") {
//                columnAttributes.autoincrement = true;
//            } else if (a == "key") {
//                columnAttributes.key = true;
//            }
//        }
////
////
//        if (std::regex_search(column_info, std::regex("[=]"))) {
//
//            column_info.erase(std::remove(column_info.begin(), column_info.end(), '='), column_info.end());
//            column_info.erase(std::remove(column_info.begin(), column_info.end(), ','), column_info.end());
//            column_info.erase(std::remove(column_info.begin(), column_info.end(), ' '), column_info.end());
//            std::cout << column_info << '\n';
//
//        } else {
//
//            if (columnType == "int32") {
//                columnObjectVariants.emplace_back(
//                        memdb::Column<int32_t>(std::move(columnName), std::move(columnAttributes), 0));
//            } else if (columnType == "bool") {
//                columnObjectVariants.emplace_back(
//                        memdb::Column<bool>(std::move(columnName), std::move(columnAttributes), 0));
//            } else {
//                std::smatch smatch1;
//                std::regex_search(columnType, smatch1, std::regex("[^\\[\\]/0-9]+"));
//                columnType = smatch1.str();
//                std::string lenght = smatch1.suffix();
//
//                std::regex_search(lenght, smatch1, std::regex("[0-9]+"));
//                lenght = smatch1.str();
//
//                int len = std::stoi(lenght);
//
//                if (columnType == "string") {
//                    columnObjectVariants.emplace_back(
//                            memdb::Column<std::string>(std::move(columnName), std::move(columnAttributes), len));
//                } else if (columnType == "bytes") {
//                    columnObjectVariants.emplace_back(
//                            memdb::Column<std::vector<uint8_t>>(std::move(columnName), std::move(columnAttributes),
//                                                                len));
//                } else {
//                    throw ExecutionException("unknown type " + columnType + "\n");
//                }
//            }
//        }
//        std::cout << "\n";
//
//    }
    bool a = 1;
    bool b = 0;
    std::cout << a ^ b;

    return 0;
}