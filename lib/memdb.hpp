#pragma once
#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <exception>
#include <iostream>

class ExecutionException : std::exception
{
public:
    ExecutionException(std::string &&whatStr) noexcept : whatStr(std::move(whatStr)) {}
    ExecutionException(const std::string &whatStr) noexcept : whatStr(whatStr) {}
    ~ExecutionException() noexcept {}
    const char* what() const noexcept override {
        return whatStr.c_str();
    }
private:
    std::string whatStr;
};


namespace memdb {

#define int int32_t

    struct ColumnAttributes {
        bool unique = false;
        bool autoincrement = false;
        bool key = false;
    };


    template <typename T>
    struct Column {
        //for ints and bools l can have any value
        Column(std::string &&s, int l) {
            name = s;
            len = l;
            type = typeid(T).name();
        }
        Column(std::string &&s, T &&v, int l) {
            name = s;
            value = v;
            type = typeid(T).name();
        };
        Column(std::string &&s, ColumnAttributes &&attr, T &&v, int l) {
            name = s;
            attributes = attr;
            value = v;
            len = l;
            type = typeid(T).name();
        };
        Column(std::string &&s, ColumnAttributes &&attr, int l) {
            name = s;
            attributes = attr;
            len = l;
            type = typeid(T).name();
        };
        Column(const Column &c) {
            name = c.name;
            attributes = c.attributes;
            len = c.getLen();
            type = c.getType();
            value = c.value;
            vector = c.vector;
            hasDefoltValue = c.hasDefoltValue;

        }
        Column(Column &&c) {
            name = c.name;
            attributes = c.attributes;
            len = c.getLen();
            type = c.getType();
            value = c.value;
            vector = c.vector;
            hasDefoltValue = c.hasDefoltValue;

        }
        void operator=(const Column &c) {
            name = c.name;
            attributes = c.attributes;
            len = c.getLen();
            type = c.getType();
            value = c.value;
            vector = c.vector;
            hasDefoltValue = c.hasDefoltValue;
        }
        void operator=(Column &&c) {
            name = c.name;
            attributes = c.attributes;
            len = c.getLen();
            type = c.getType();
            value = c.value;
            vector = c.vector;
            hasDefoltValue = c.hasDefoltValue;
        }
        std::vector<T> vector;
        T value;
        bool hasDefoltValue = false;
        std::string name;
        ColumnAttributes attributes;
        int getLen() const {return len;}
        std::string getType() const {return type;}
    private:
        std::string type;
        int len; // for strings and bytes
    };

    template <typename T>
    struct LineValue {
        T value;
        bool defaultValue;
    };

    struct ByteString {
        std::string str;
    };

    struct Line {
        Line(bool has_names, std::vector<std::pair<std::string, std::variant<LineValue<int>, LineValue<bool>, LineValue<std::string>, LineValue<ByteString>>>> &&vec) {
            hasNames = has_names;
            values = std::move(vec);
        }
        std::vector<std::pair<std::string, std::variant<LineValue<int>, LineValue<bool>, LineValue<std::string>, LineValue<ByteString>>>> values;
        bool hasNames = false;
    };

    enum ColumnTypes{
        C_INT= 0,
        C_BOOL=1,
        C_STRING=2,
        C_BYTE=3
    };


    struct Table {
        std::vector<std::variant<Column<int>, Column<bool>, Column<std::string>, Column<ByteString>>> columns;
        std::string name;
        int getColumnsCount() const {return columns.size();}
        Table(std::string &&s, std::vector<std::variant<Column<int>, Column<bool>, Column<std::string>, Column<ByteString>>> &&vec) {
            name = s;
            columns = std::move(vec);
        }

        Table(const Table &t) {
            columns = t.columns;
            name = t.name;
        }
        Table(Table &&t) {
            columns = t.columns;
            name = t.name;
        }
        void operator=(const Table& t) {
            columns = t.columns;
            name = t.name;
        }
        void operator=(Table&& t) {
            columns = t.columns;
            name = t.name;
        }

        std::string getColumnsNamesString() {
            std::string s;
            for (auto &c : columns) {
                switch (c.index()) {
                    case C_INT: {
                        s += std::get<C_INT>(c).name + "|";
                        break;
                    }
                    case C_BOOL: {
                        s += std::get<C_BOOL>(c).name + "|";
                        break;
                    }
                    case C_BYTE: {
                        s += std::get<C_BYTE>(c).name + '|';
                        break;
                    }
                    case C_STRING: {
                        s += std::get<C_STRING>(c).name + '|';
                        break;
                    }
                }
            }
            return s;
        }

        void insert(Line line) {
            if (line.values.size() > columns.size()) throw ExecutionException("inserting more arguments than expected\n");

            if (line.hasNames) {
                for (auto &val : line.values) {
                    bool found = false;
                    for (auto column_var : columns) {

                        switch(column_var.index()){
                            case C_INT: {
                                auto column = std::get<C_INT>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(val.second) != typeid(int)) {
                                        throw ExecutionException(
                                                "invalid type inserting in column " + val.first + "\n");
                                    }

                                    if (std::get<C_INT>(val.second).defaultValue) {
                                        if (!column.hasDefoltValue) {
                                            throw ExecutionException(
                                                    "column " + column.name + " has not default value\n");
                                        }

                                        column.vector.push_back(column.value);
                                    } else {
                                        column.vector.push_back(std::get<C_INT>(val.second).value);
                                    }

                                    found = true;
                                }
                                break;
                            }
                            case C_BOOL: {
                                auto column = std::get<C_BOOL>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(val.second) != typeid(bool)) {
                                        throw ExecutionException(
                                                "invalid type inserting in column " + val.first + "\n");
                                    }

                                    if (std::get<C_BOOL>(val.second).defaultValue) {
                                        if (!column.hasDefoltValue) {
                                            throw ExecutionException(
                                                    "column " + column.name + " has not default value\n");
                                        }

                                        column.vector.push_back(column.value);
                                    } else {
                                        column.vector.push_back(std::get<C_BOOL>(val.second).value);
                                    }

                                    found = true;
                                }
                                break;
                            }
                            case C_STRING: {
                                auto column = std::get<C_STRING>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(val.second) != typeid(std::string)) {
                                        throw ExecutionException(
                                                "invalid type inserting in column " + val.first + "\n");
                                    }

                                    if (std::get<C_STRING>(val.second).defaultValue) {
                                        if (!column.hasDefoltValue) {
                                            throw ExecutionException(
                                                    "column " + column.name + " has not default value\n");
                                        }

                                        column.vector.push_back(column.value);
                                    } else {
                                        if (std::get<C_STRING>(val.second).value.size() > column.getLen()) {
                                            throw ExecutionException("inserting too long string in column " + val.first + "\n");
                                        }
                                        column.vector.push_back(std::get<C_STRING>(val.second).value);
                                    }

                                    found = true;
                                }
                                break;
                            }
                            case C_BYTE: {
                                auto column = std::get<C_BYTE>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(val.second) != typeid(ByteString)) {
                                        throw ExecutionException(
                                                "invalid type inserting in column " + val.first + "\n");
                                    }

                                    if (std::get<C_BYTE>(val.second).defaultValue) {
                                        if (!column.hasDefoltValue) {
                                            throw ExecutionException(
                                                    "column " + column.name + " has not default value\n");
                                        }

                                        column.vector.push_back(column.value);
                                    } else {
                                        if (std::get<C_BYTE>(val.second).value.str.size() != column.getLen()) {
                                            throw ExecutionException("invalid byte array lenght inserting in column " + val.first + "\n");
                                        }
                                        column.vector.push_back(std::get<C_BYTE>(val.second).value);
                                    }

                                    found = true;
                                }
                                break;
                            }
                            default: {
                                throw ExecutionException("error while column type definition (unexpected) \n");
                            }
                        }
                        if (found) break;
                    }

                    if (!found) throw ExecutionException("no column with name " + val.first + "\n");
                }
            } else {
                for (int i = 0; i < line.values.size(); ++i) {
                    auto val = line.values[i];
                    auto column_var = columns[i];

                    switch(column_var.index()) {
                        case C_INT: {
                            auto column = std::get<C_INT>(column_var);
                            if (std::get<C_INT>(val.second).defaultValue) {
                                if (!column.hasDefoltValue) {
                                    throw ExecutionException("column " + column.name + " as not default value\n");
                                }

                                column.vector.push_back(column.value);
                            } else {
                                if (typeid(std::get<C_INT>(val.second).value) != typeid(int)) {
                                    throw ExecutionException("invalid type inserting in column " + column.name + "\n");
                                }

                                column.vector.push_back(std::get<C_INT>(val.second).value);
                            }
                            break;
                        }
                        case C_BOOL: {
                            auto column = std::get<C_BOOL>(column_var);
                            if (std::get<C_BOOL>(val.second).defaultValue) {
                                if (!column.hasDefoltValue) {
                                    throw ExecutionException("column " + column.name + " as not default value\n");
                                }

                                column.vector.push_back(column.value);
                            } else {
                                if (typeid(std::get<C_BOOL>(val.second).value) != typeid(bool)) {
                                    throw ExecutionException("invalid type inserting in column " + column.name + "\n");
                                }

                                column.vector.push_back(std::get<C_BOOL>(val.second).value);
                            }
                            break;
                        }
                        case C_STRING: {
                            auto column = std::get<C_STRING>(column_var);
                            if (std::get<C_STRING>(val.second).defaultValue) {
                                if (!column.hasDefoltValue) {
                                    throw ExecutionException("column " + column.name + " as not default value\n");
                                }

                                column.vector.push_back(column.value);
                            } else {
                                if (typeid(std::get<C_STRING>(val.second).value) != typeid(std::string)) {
                                    throw ExecutionException("invalid type inserting in column " + column.name + "\n");
                                }

                                if (std::get<C_STRING>(val.second).value.size() > column.getLen()) {
                                    throw ExecutionException("trying to insert too long string to column " + column.name + "\n");
                                }

                                column.vector.push_back(std::get<C_STRING>(val.second).value);
                            }
                            break;
                        }
                        case C_BYTE: {
                            auto column = std::get<C_BYTE>(column_var);
                            if (std::get<C_BYTE>(val.second).defaultValue) {
                                if (!column.hasDefoltValue) {
                                    throw ExecutionException("column " + column.name + " as not default value\n");
                                }

                                column.vector.push_back(column.value);
                            } else {
                                if (typeid(std::get<C_BYTE>(val.second).value) != typeid(ByteString)) {
                                    throw ExecutionException("invalid type inserting in column " + column.name + "\n");
                                }

                                if (std::get<C_BYTE>(val.second).value.str.size() != column.getLen()) {
                                    throw ExecutionException("trying to insert byte array with invalid lenght to column " + column.name + "\n");
                                }

                                column.vector.push_back(std::get<C_BYTE>(val.second).value);
                            }
                            break;
                        }
                        default: {
                            throw ExecutionException("error while column type definition (unexpected) \n");
                        }
                    }
                }
            }
        }

    };




#include <iostream>
class Database {
    std::vector<Table> tables;

public:

        std::string getTablesNamesString() {
            std::string s;
            for (auto &t : tables) {
                s += t.name += '|';
            }
            return s;
        }
        std::string getTablesColumnsString() {
            std::string s;
            for (auto &t : tables) {
                s += (t.getColumnsNamesString());
            }
            return s;
        }
        int getAllColumnsCount() {
            int i = 0;
            for (auto &t : tables) {
                i += t.getColumnsCount();
            }
            return i;
        }
        void addNewTable(Table &&t) {
            tables.push_back(t);
        }
        void insertToTable(std::string &&name, Line &&line) {
            bool found = false;
            for (auto table : tables) {
                if (table.name == name) {
                    table.insert(line);
                    found = true;
                    break;
                }
            }
            if (!found) throw ExecutionException("no tables with name " + name + "\n");
        }
    };
#undef int
}




