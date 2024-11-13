#pragma once
#include <string>
#include <memory>
#include <vector>
#include <variant>

class ExecutionException : std::exception
{
public:
    ExecutionException(std::string &&whatStr) noexcept : whatStr(std::move(whatStr)) {}
    ExecutionException(const std::string &whatStr) noexcept : whatStr(whatStr) {}
    ~ExecutionException() noexcept;
    const char* what() const noexcept override;
private:
    std::string whatStr;
};

const char* ExecutionException::what() const noexcept
{
    return whatStr.c_str();
}

namespace memdb {

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
        int getLen() {return len;}
    private:
        std::string type;
        int len; // for strings and bytes
    };

    template <typename T>
    struct LineValue {
        T value;
        bool defaultValue;
    };

    struct Line {
        Line(bool has_names, std::vector<std::pair<std::string, std::variant<LineValue<int>, LineValue<bool>, LineValue<std::string>, LineValue<std::vector<uint8_t>>>>> &&vec) {
            hasNames = has_names;
            values = std::move(vec);
        }
        std::vector<std::pair<std::string, std::variant<LineValue<int>, LineValue<bool>, LineValue<std::string>, LineValue<std::vector<uint8_t>>>>> values;
        bool hasNames = false;
    };

    enum ColumnTypes{
        C_INT= 0,
        C_BOOL=1,
        C_STRING=2,
        C_BYTE=3
    };

    struct Table {
        std::vector<std::variant<Column<int>, Column<bool>, Column<std::string>, Column<std::vector<uint8_t>>>> columns;
        std::string name;
        int getColumnsCount() {return columns.size();}
        Table(std::string &&s, std::vector<std::variant<Column<int>, Column<bool>, Column<std::string>, Column<std::vector<uint8_t>>>> &&vec) {
            name = s;
            columns = std::move(vec);
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
                                    if (typeid(val.second) != typeid(std::vector<uint8_t>)) {
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
                                        if (std::get<C_BYTE>(val.second).value.size() != column.getLen()) {
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
                                if (typeid(std::get<C_BYTE>(val.second).value) != typeid(std::vector<uint8_t>)) {
                                    throw ExecutionException("invalid type inserting in column " + column.name + "\n");
                                }

                                if (std::get<C_BYTE>(val.second).value.size() != column.getLen()) {
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


//    class Database {
//        std::vector<Table> tables;
//        std::vector<const std::type_info*> types;
//
//    public:
//        void addNewTable(const std::shared_ptr<TableBase>&, const std::type_info *);
//        void insertToTable(std::string &values, std::string &name);
//    };

}




