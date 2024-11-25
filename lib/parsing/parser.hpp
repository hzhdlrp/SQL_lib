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
        throw ExecutionException("can not find table with name " + name + "\n");
        return std::monostate{};
    }
};

class Select : public Query {
    std::string table_name;
    std::vector<std::string> columns_names;
    std::string condition;

public:
    Select(std::string &tn, std::vector<std::string> &cn, std::string &cnd) : table_name(tn), columns_names(cn), condition(cnd) {}
    std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) override {
        for (auto &t : db.tables) {
            if (t.name == table_name) {
                return t.select(columns_names, condition);
            }
        }
        throw ExecutionException("can not find table with name " + table_name + "\n");
    }
};

std::variant<memdb::Table, std::monostate> execute(memdb::Database &db, const std::shared_ptr<Query>& q) {
    return q->execute(db);
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
                return createTableQueryParse(ss);
            }
        }
        else if (command_name == "insert") {
            std::string remained = ss.str();
            return insertQueryParse(remained);
        }

        else if (command_name == "select") {
            std::string remained = ss.str();

            size_t start {remained.find(command_name)};
            remained.erase(start, command_name.size());
            return selectQueryParse(remained);
        }
    }
private:
    std::shared_ptr<Query> insertQueryParse(std::string &remained) {
        std::regex r(R"(\(([^()]+)\))");
        std::string in_braces;
        bool by_name = false;

        for (std::smatch sm; regex_search(remained, sm, r);)
        {
            in_braces = sm.str() ;
            remained = sm.suffix();
        }
        if (in_braces.find('=')) {
            by_name = true;
        }

        if (by_name) {
            return namedInsert(in_braces, remained);
        } else {
            return unnamedInsert(in_braces, remained);
        }
    }

    std::shared_ptr<Query> createTableQueryParse(std::stringstream &ss) {
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
                    auto __col = memdb::Column<int32_t>(std::move(columnName), std::move(columnAttributes), std::move(val), 0);
                    __col.hasDefoltValue = true;
                    columnObjectVariants.emplace_back(__col);
                } else if (columnType == "bool") {
                    bool val = (column_info == "true");
                    auto __col = memdb::Column<bool>(std::move(columnName), std::move(columnAttributes), std::move(val),0);
                    __col.hasDefoltValue = true;
                    columnObjectVariants.emplace_back(__col);
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
                        auto __col = memdb::Column<std::string>(std::move(columnName), std::move(columnAttributes), std::move(column_info), len);
                        __col.hasDefoltValue = true;
                        columnObjectVariants.emplace_back(__col);
                    } else if (columnType == "bytes") {
                        if (column_info[0] != '0' || column_info[1] != 'x') throw ExecutionException("byte value must brgin with 0x\n");
                        if (column_info.size() != len + 2) throw ExecutionException("incorrect lenght of default byte value\n");
                        auto __col = memdb::Column<memdb::ByteString>(std::move(columnName), std::move(columnAttributes), memdb::ByteString(column_info),len);
                        __col.hasDefoltValue = true;
                        columnObjectVariants.emplace_back(__col);
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

        memdb::Table table(std::move(name), std::move(columnObjectVariants));
        CreateTable createTable(table);
        return std::make_shared<CreateTable>(createTable);
    }

    std::shared_ptr<Query> selectQueryParse(std::string &remained) {
        std::string columns_list;
        for (std::smatch sm; regex_search(remained, sm, std::regex("from"));) {
            columns_list = sm.prefix();
            remained = sm.suffix();
            break;
        }

        std::vector<std::string> columns_names;
        for (std::smatch sm; regex_search(columns_list, sm, std::regex(","));) {
            columns_names.push_back(sm.prefix());
            columns_list = sm.suffix();
        }

        for (auto &s: columns_names) {
            s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
        }

        std::string table_name;
        std::string condition;
        for (std::smatch sm; regex_search(remained, sm, std::regex("where"));) {
            table_name = sm.prefix();
            condition = sm.suffix();
            break;
        }
        table_name.erase(std::remove(table_name.begin(), table_name.end(), ' '), table_name.end());

        Select select(table_name, columns_names, condition);
        return std::make_shared<Select>(select);
    }

    std::shared_ptr<Query> namedInsert(std::string &in_braces, std::string &remained) {
        std::vector<std::string> names_values;
        for (std::smatch sm; regex_search(in_braces, sm, std::regex("[^,\\(\\)]+")); ) {
            names_values.push_back(sm.str());
            in_braces = sm.suffix();
        }
        std::vector<std::string> names;
        std::vector<std::string> values;

        for (auto &s : names_values) {
            for (std::smatch sm; regex_search(s, sm, std::regex("=")); ) {
                names.push_back(sm.prefix());
                values.push_back(sm.suffix());
                break;
            }
        }
        for (int i = 0; i < names.size(); ++i) {
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

        Insert insert{table_name, line};
        return std::make_shared<Insert>(insert);
    }

    std::shared_ptr<Query> unnamedInsert(std::string &in_braces, std::string &remained) {
        in_braces.erase(std::remove(in_braces.begin(), in_braces.end(), '('), in_braces.end());
        in_braces.erase(std::remove(in_braces.begin(), in_braces.end(), ')'), in_braces.end());

        std::vector<std::string> values;
        int cntr = 0;
        for (std::smatch sm; regex_search(in_braces, sm, std::regex(",")); ) {
            values.push_back(sm.prefix());
            in_braces = sm.suffix();
        }

        for (auto &s:values) {
            std::cout << s << " ";
        } std::cout << '\n';
        using  LineVariant = std::variant<memdb::LineValue<int>, memdb::LineValue<bool>, memdb::LineValue<std::string>, memdb::LineValue<memdb::ByteString>>;
        std::vector<std::pair<std::string, LineVariant>> vec;

        for (auto &s : values) {
            s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
            if (s.empty()) {
                vec.push_back(std::pair("", LineVariant(memdb::LineValue<int>(0, true))));
                break;
            }
            if ((s[0] >= '0' && s[0] <= '9' && s.size() == 1) || (s[0] >= '0' && s[0] <= '9' && s[1] != 'x')) {
                vec.push_back(std::pair("", LineVariant(memdb::LineValue<int>(std::stoi(s), false))));
                break;
            }
            if (s[0] == '\"') {
                s.erase(std::remove(s.begin(), s.end(), '\"'), s.end());
                vec.push_back(std::pair("", LineVariant(memdb::LineValue<std::string>(s, false))));
                break;
            }
            if (s.size() >2 && s[0] == '0' && s[1] == 'x') {
                vec.push_back(std::pair("", LineVariant(memdb::LineValue<memdb::ByteString>(memdb::ByteString(s), false))));
                break;
            }
            throw ExecutionException("unknown type of value " + s + "\n");
        }

        memdb::Line line(false, std::move(vec));
        std::stringstream ss(remained);
        std::string table_name;
        ss >> table_name >> table_name;

        Insert insert{table_name, line};
        return std::make_shared<Insert>(insert);

    }
};