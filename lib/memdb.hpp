#pragma once
#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <exception>
#include <iostream>
#include <map>
#include <fstream>

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

        void insert(Line &line) {
            if (line.values.size() > columns.size()) throw ExecutionException("inserting more arguments than expected\n");
            if (line.hasNames) {
                std::map<int, bool> isUsedColumnNamesByIndex;
                for (int i = 0; i < columns.size(); ++i) {
                    isUsedColumnNamesByIndex[i] = false;
                }
                for (auto &val : line.values) {
                    bool found = false;
                    for (int index = 0; index < columns.size(); ++index) {
                        auto &column_var = columns[index];
                        switch(column_var.index()){
                            case C_INT: {
                                auto &column = std::get<C_INT>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(std::get<C_INT>(val.second).value) != typeid(int)) {
                                        throw ExecutionException(
                                                "invalid type inserting in column " + val.first + "\n");
                                    }

                                    if (std::get<C_INT>(val.second).defaultValue) {
                                        if (!column.hasDefoltValue) {
                                            throw ExecutionException(
                                                    "column " + column.name + " has not default value\n");
                                        }

                                        column.vector.push_back(column.value);
                                        ///TODO(autoinc)
                                    } else {
                                        column.vector.push_back(std::get<C_INT>(val.second).value);
                                    }

                                    found = true;
                                    isUsedColumnNamesByIndex[index] = true;
                                }
                                break;
                            }
                            case C_BOOL: {
                                auto &column = std::get<C_BOOL>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(std::get<C_BOOL>(val.second).value) != typeid(bool)) {
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
                                    isUsedColumnNamesByIndex[index] = true;
                                }
                                break;
                            }
                            case C_STRING: {
                                auto &column = std::get<C_STRING>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(std::get<C_STRING>(val.second).value) != typeid(std::string)) {
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

                                    isUsedColumnNamesByIndex[index] = true;
                                    found = true;
                                }
                                break;
                            }
                            case C_BYTE: {
                                auto &column = std::get<C_BYTE>(column_var);
                                if (column.name == val.first) {
                                    if (typeid(std::get<C_BYTE>(val.second).value) != typeid(ByteString)) {
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

                                    isUsedColumnNamesByIndex[index] = true;
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

                for (auto &pair : isUsedColumnNamesByIndex) {
                    if (pair.second == false) {
                        auto &column_var = columns[pair.first];
                        switch (column_var.index()) {
                            case C_INT:{
                                auto &col = std::get<C_INT>(column_var);
                                if (!col.hasDefoltValue) {
                                    if (!col.attributes.autoincrement) {
                                        throw ExecutionException(
                                                col.name + " has not default value and autoincrement\n");
                                    } else {

                                        if (col.vector.empty()) {
                                            col.vector.push_back(0);

                                        } else {
                                            col.vector.push_back(col.vector.back() + 1);
                                        }
                                    }
                                } else {
                                    col.vector.push_back(col.value);
                                }
                                break;
                            }
                            case C_BOOL:{
                                auto &col = std::get<C_BOOL>(column_var);
                                if (!col.hasDefoltValue) {

                                        throw ExecutionException(
                                                col.name + " has not default value and autoincrement\n");

                                } else {
                                    col.vector.push_back(col.value);
                                }
                                break;

                            }
                            case C_STRING:{
                                auto &col = std::get<C_STRING>(column_var);
                                if (!col.hasDefoltValue) {

                                    throw ExecutionException(
                                            col.name + " has not default value and autoincrement\n");

                                } else {
                                    col.vector.push_back(col.value);
                                }
                                break;

                            }
                            case C_BYTE:{
                                auto &col = std::get<C_BYTE>(column_var);
                                if (!col.hasDefoltValue) {

                                    throw ExecutionException(
                                            col.name + " has not default value and autoincrement\n");

                                } else {
                                    col.vector.push_back(col.value);
                                }
                                break;

                            }
                        }
                    }
                }
            } else {
                for (int i = 0; i < line.values.size(); ++i) {
                    auto val = line.values[i];
                    auto &column_var = columns[i];

                    switch(column_var.index()) {
                        case C_INT: {
                            auto &column = std::get<C_INT>(column_var);
                            if (std::get<C_INT>(val.second).defaultValue) {
                                if (!column.hasDefoltValue) {
                                    throw ExecutionException("column " + name + " as not default value\n");
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
                            auto &column = std::get<C_BOOL>(column_var);
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
                            auto &column = std::get<C_STRING>(column_var);
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
                            auto &column = std::get<C_BYTE>(column_var);
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






    struct Database {
        std::vector<Table> tables;

        void uploadToFile(std::string &&file) {

            std::ofstream out;
            out.open(file, std::ios::app);
            std::cout << file << '\n';
            if (out.is_open()) {
                for (auto &t : tables) {
                    out <<  "TABLE  " << t.name << " :\n\n";
                    for (auto &cv : t.columns) {
                        switch (cv.index()) {
                            case C_INT: {
                                auto &c = std::get<C_INT>(cv);
                                out << "COLUMN " << c.name << " (type int32):\n";
                                for (auto &i : c.vector) {
                                    out << i << '\n';
                                }
                                out << '\n';
                                break;
                            }
                            case C_BOOL: {
                                auto &c = std::get<C_BOOL>(cv);
                                out << "COLUMN " << c.name << " (type bool):\n";
                                for (auto i : c.vector) {
                                    out << i << '\n';
                                }
                                out << '\n';
                                break;
                            }
                            case C_STRING: {
                                auto &c = std::get<C_STRING>(cv);
                                out << "COLUMN " << c.name << " (type string[" <<c.getLen()<< "]):\n";
                                for (auto &i : c.vector) {
                                    out << i << '\n';
                                }
                                out << '\n';
                                break;
                            }
                            case C_BYTE: {
                                auto &c = std::get<C_BYTE>(cv);
                                out << "COLUMN " << c.name << " (type byte[" <<c.getLen()<< "]):\n";
                                for (auto &i : c.vector) {
                                    out << i.str << '\n';
                                }
                                out << '\n';
                                break;
                            }
                        }
                    }
                    out << '\n';
                }
            } else {
                throw ExecutionException("can not open file" + file + "\n");
            }
            out.close();
        }


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




