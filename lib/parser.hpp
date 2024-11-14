#pragma once
#include "memdb.hpp"
#include "memory"
#include <variant>
#include <sstream>
#include "algorithm"
#include <regex>
#include <set>

struct Query {
public:
    virtual std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) = 0;
};

struct Create : public Query {
    memdb::Table table;
public:
    Create(memdb::Table &t) : table(t) {}

    std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) override {
        db.addNewTable(std::move(table));
        return std::monostate{};
    }
};

struct Insert : public Query {
    std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) override {

    }
};

void execute(memdb::Database &db, Query &&q) {
    q.execute(db);
}

std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); }
    );
    return s;
}



class Parser {
    std::shared_ptr<Query> parse(std::string &&string) {
        std::stringstream ss{string};
        std::string command_name;
        ss >> command_name;
        command_name = str_tolower(command_name);

        if (command_name == "create") {
            std::string subcommand_name;
            ss >> subcommand_name;
            subcommand_name = str_tolower(subcommand_name);

            if (subcommand_name == "table") {
                std::string name;
                ss >> name;

                std::regex r(R"((?:\{[^}]+\}\s*)?\w+\s*:\s*\w+(?:\[\d+\])?(?:\s*=\s*[^,)]+)?,?)");
                std::string remained;
                ss >> remained;

                std::vector<std::string> columns_info;
                std::vector<std::variant<memdb::Column<int>, memdb::Column<bool>, memdb::Column<std::string>, memdb::Column<std::vector<uint8_t>>>> columnObjectVariants;
                for (std::smatch sm; regex_search(remained, sm, r);)
                {
                    columns_info.push_back(sm.str());
                    remained= sm.suffix();
                }

                for (auto &column_info : columns_info) {
                    std::vector<std::string> attributes;
                    std::regex r1(R"((?:\{[^}]+\}\s*)?)");
                    std::string attr;
                    std::smatch smatch;
                    std::regex_search(column_info, smatch, r1);
                    attr = smatch.str();

                    column_info = smatch.suffix();

                    for (std::smatch sm; regex_search(attr, sm, std::regex(R"([^,{}\s]+)"));)
                    {
                        attributes.push_back(str_tolower(sm.str()));
                        attr = sm.suffix();
                    }

                    memdb::ColumnAttributes columnAttributes;
                    std::set<std::string> attributesSet = {"unique", "autoincrement", "key"};

                    for (auto &a : attributes) {
                        if (!attributesSet.contains(a)) {
                            throw ExecutionException("unknown attribute\n");
                        }

                        if (a == "unique") {
                            columnAttributes.unique = true;
                        } else if (a == "autoincrement") {
                            columnAttributes.autoincrement = true;
                        } else if (a == "key") {
                            columnAttributes.key = true;
                        }
                    }

                    std::string columnName;
                    std::regex_search(column_info, smatch, std::regex(R"([^\s:]+)"));

                    columnName = smatch.str();
                    column_info = smatch.suffix();

                    std::string columnType;
                    std::regex_search(column_info, smatch, std::regex(R"([^\s:=])"));

                    columnType = smatch.str();
                    column_info = smatch.suffix();

                    if (std::regex_search(column_info, std::regex("[=]"))) {

                    } else {
                        if (columnType == "int32") {
                            columnObjectVariants.emplace_back(memdb::Column<int32_t>(std::move(columnName), std::move(columnAttributes), 0));
                        } else if (columnType == "bool") {
                            columnObjectVariants.emplace_back(memdb::Column<bool>(std::move(columnName), std::move(columnAttributes), 0));
                        } else {
                            std::smatch smatch1;
                            std::regex_search(columnType, smatch1, std::regex("[^\\[\\]/0-9]+"));
                            columnType = smatch1.str();
                            std::string lenght = smatch1.suffix();

                            std::regex_search(lenght, smatch1, std::regex("[0-9]+"));
                            lenght = smatch1.str();

                            int len = std::stoi(lenght);

                            if (columnType == "string") {
                                columnObjectVariants.emplace_back(memdb::Column<std::string>(std::move(columnName), std::move(columnAttributes), len));
                            } else if (columnType == "bytes") {
                                columnObjectVariants.emplace_back(memdb::Column<std::vector<uint8_t>>(std::move(columnName), std::move(columnAttributes), len));
                            } else {
                                throw ExecutionException("unknown type " + columnType + "\n");
                            }
                        }
                    }
                }

            }
        }
    }
};