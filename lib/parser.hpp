#pragma once
#include "memdb.hpp"
#include <memory>
#include <variant>
#include <sstream>
#include <algorithm>
#include <regex>
#include <set>

struct Query {
public:
    virtual std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) = 0;
};

class CreateTable : public Query {
    memdb::Table table;
public:
    CreateTable(memdb::Table &t) : table(t) {}

    std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) override {
        db.addNewTable(std::move(table));
        return std::monostate{};
    }
};

class Insert : public Query {
    std::string name;
    memdb::Line line;
public:
    Insert(std::string  &s, memdb::Line &l) : name(s), line(l) {}
    std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) override {
        for (auto &t : db.tables) {
            if (t.name == name) {
                t.insert(line);
                return std::monostate{};
            }
        }
        return std::monostate{};
    }
};

void execute(memdb::Database &db, const std::shared_ptr<Query>& q) {
    q->execute(db);
}

std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); }
    );
    return s;
}

struct Parser {
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
                std::string remained = ss.str();

                std::vector<std::string> columns_info;
                std::vector<std::variant<memdb::Column<int>, memdb::Column<bool>, memdb::Column<std::string>, memdb::Column<memdb::ByteString>>> columnObjectVariants;
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
                            throw ExecutionException("unknown attribute " + a + "\n");
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
                    std::regex_search(column_info, smatch, std::regex(R"([^\s:=]+)"));

                    columnType = smatch.str();
                    column_info = smatch.suffix();

                    if (std::regex_search(column_info, std::regex("[=]"))) {

                        column_info.erase(std::remove(column_info.begin(), column_info.end(), '='), column_info.end());
                        column_info.erase(std::remove(column_info.begin(), column_info.end(), ','), column_info.end());
                        column_info.erase(std::remove(column_info.begin(), column_info.end(), ' '), column_info.end());


                        if (columnType == "int32") {
                            int32_t val;
                            std::stringstream ss1(column_info);
                            ss1 >> val;
                            columnObjectVariants.emplace_back(memdb::Column<int32_t>(std::move(columnName), std::move(columnAttributes), std::move(val), 0));
                        } else if (columnType == "bool") {
                            bool val = (column_info == "true");
                            columnObjectVariants.emplace_back(memdb::Column<bool>(std::move(columnName), std::move(columnAttributes), std::move(val),0));
                        } else {
                            std::smatch smatch1;

                            std::regex_search(columnType, smatch1, std::regex("[^\\[\\]/0-9]+"));
                            columnType = smatch1.str();
                            std::string lenght = smatch1.suffix();

                            std::regex_search(lenght, smatch1, std::regex("[0-9]+"));
                            lenght = smatch1.str();

                            int len = std::stoi(lenght);

                            if (columnType == "string") {
                                column_info.erase(std::remove(column_info.begin(), column_info.end(), '='), column_info.end());
                                if (column_info.size() > len) throw ExecutionException("incorrect lenght of default string value\n");
                                columnObjectVariants.emplace_back(memdb::Column<std::string>(std::move(columnName), std::move(columnAttributes), std::move(column_info), len));
                            } else if (columnType == "bytes") {
                                if (column_info[0] != '0' || column_info[1] != 'x') throw ExecutionException("byte value must brgin with 0x\n");
                                if (column_info.size() != len + 2) throw ExecutionException("incorrect lenght of default byte value\n");
                                columnObjectVariants.emplace_back(memdb::Column<memdb::ByteString>(std::move(columnName), std::move(columnAttributes), memdb::ByteString(column_info),len));
                            } else {
                                throw ExecutionException("unknown type " + columnType + "\n");
                            }
                        }
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
                                columnObjectVariants.emplace_back(memdb::Column<memdb::ByteString>(std::move(columnName), std::move(columnAttributes), len));
                            } else {
                                throw ExecutionException("unknown type " + columnType + "\n");
                            }
                        }
                    }

                }

//                std::cout << columnObjectVariants.size();
                memdb::Table table(std::move(name), std::move(columnObjectVariants));
                CreateTable createTable(table);
                return std::make_shared<CreateTable>(createTable);
            }
        }
        else if (command_name == "insert") {
            std::string remained = ss.str();
            std::regex r(R"(\(([^()]+)\))");
            std::vector<std::string> names_values;
            std::string in_braces;
            bool by_name = false;

            for (std::smatch sm; regex_search(remained, sm, r);)
            {
                in_braces = sm.str() ;
                remained = sm.suffix();
            }
            std::cout << "in (): " << in_braces << "\nremained = " << remained << "\n\n";

            if (in_braces.find('=')) {
                by_name = true;
            }

            for (std::smatch sm; regex_search(in_braces, sm, std::regex("[^,\\(\\)]+")); ) {
                names_values.push_back(sm.str());
                std::cout << sm.str() << '\n';
                in_braces = sm.suffix();
            }

            if (by_name) {
                std::vector<std::string> names;
                std::vector<std::string> values;

                for (auto &s : names_values) {
                    std::cout << s << '\n';
                    for (std::smatch sm; regex_search(s, sm, std::regex("=")); ) {
                        names.push_back(sm.prefix());
                        values.push_back(sm.suffix());
                        break;
                    }
                }
                for (int i = 0; i < names.size(); ++i) {
                    std::cout << "name = " << names[i] << "; value = " << values[i] << '\n';
                } std::cout << '\n';

                using  LineVariant = std::variant<memdb::LineValue<int>, memdb::LineValue<bool>, memdb::LineValue<std::string>, memdb::LineValue<memdb::ByteString>>;
                std::vector<std::pair<std::string, LineVariant>> vec;
                for (int i = 0; i < names.size(); ++i) {
                    values[i].erase(std::remove(values[i].begin(), values[i].end(), ' '), values[i].end());
                    names[i].erase(std::remove(names[i].begin(), names[i].end(), ' '), names[i].end());

                    if (values[i][0] == '\"') {
                        values[i].erase(std::remove(values[i].begin(), values[i].end(), '\"'), values[i].end());

                        LineVariant variant(memdb::LineValue<std::string>(values[i], false));
                        vec.push_back(std::pair(names[i], variant));
                    } else if (values[i][0] == '0' && values[i][1] == 'x') {
                        LineVariant variant(memdb::LineValue<memdb::ByteString>({values[i]}, false));
                        vec.push_back(std::pair(names[i], variant));
                    } else if (values[i] == "true") {
                        LineVariant variant(memdb::LineValue<bool>(true, false));
                        vec.push_back(std::pair(names[i], variant));
                    } else if (values[i] == "false") {
                        LineVariant variant(memdb::LineValue<bool>(false, false));
                        vec.push_back(std::pair(names[i], variant));
                    } else {
                        for (auto &c : values[i]) {
                            if (c < '0' || c > '9') {
                                throw ExecutionException("unknown type of value " + values[i] + "\n");
                            }
                            LineVariant variant(memdb::LineValue<int32_t>(std::stoi(values[i]), false));
                            vec.push_back(std::pair(names[i], variant));
                        }
                    }


                }

                memdb::Line line(true, std::move(vec));
                std::stringstream ss(remained);
                std::string table_name;
                ss >> table_name >> table_name;
                std::cout << "table name: " << table_name << '\n';

                Insert insert{table_name, line};
                return std::make_shared<Insert>(insert);
            }



        }
    }
};