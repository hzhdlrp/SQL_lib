#pragma once
#include <string>
#include <memory>
#include <vector>
#include <variant>

namespace memdb {

    struct ColumnAttributes {
        bool unique = false;
        bool autoincrement = false;
        bool key = false;
    };

    struct ColumnInterface {
        std::string name;
        ColumnAttributes attributes;
        ColumnInterface(std::string &);
        ColumnInterface(std::string &, ColumnAttributes &);
        int number;
    private:
        virtual void foo() = 0;
    };

    template <typename T>
    struct Column : public ColumnInterface {
        Column(std::string &&, int len);
        Column(std::string &&, T &&, int len);
        Column(std::string &&, ColumnAttributes &&attr, T &&, int len);
        Column(std::string &&, ColumnAttributes &&, int len);
        std::vector<T> vector;
        T value;
        bool hasDefoltValue = false;
    private:
        void foo() override {};
        int len; // for strings and bytes
    };

    template <typename T>
    struct LineValue {
        T value;
        bool defaultValue;
    };

    template<int ArgumentsCount, typename... Types>
    struct Line {
        Line(bool, std::tuple<std::pair<std::string, Types>...> &);
        std::tuple<std::pair<std::string, LineValue<Types>>...> values;
        bool hasNames = false;
        int count = ArgumentsCount;
    };

    struct Table {
        int columnsCount = 0;
        std::vector<Column<int32_t>> intColumns;
        std::vector<Column<bool>> boolColumns;
        std::vector<Column<std::string>> stringColumns;
        std::vector<Column<std::vector<uint8_t>>> byteColumns;
        std::string name;
        int getColumnsCount() {return columnsCount;}
        Table(std::string &&, std::vector<Column<int32_t>> &&, std::vector<Column<std::string>> &&, std::vector<Column<bool>> &&, std::vector<Column<std::vector<uint8_t>>> &&);


        template<int ArgumentsCount, typename... Types>
        void insert(Line<ArgumentsCount, Types...> &&);

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




memdb::ColumnInterface::ColumnInterface(std::string &s) {name = s;}
memdb::ColumnInterface::ColumnInterface(std::string &s, ColumnAttributes &a) {name = s; attributes = a;}

template <typename T>
memdb::Column<T>::Column(std::string &&s, int lenght) : ColumnInterface(s) {len = lenght;}

template<typename T>
memdb::Column<T>::Column(std::string &&s, ColumnAttributes && a, int lenght) : ColumnInterface(s, a) {len = lenght;}

template <typename T>
memdb::Column<T>::Column(std::string &&s, T &&v, int lenght) :  ColumnInterface(s) {value = v; hasDefoltValue = true; len=lenght;}

template<typename T>
memdb::Column<T>::Column(std::string &&s, ColumnAttributes &&a, T &&v, int lenght)  : ColumnInterface(s, a) {value = v; hasDefoltValue = true; len = lenght;}


memdb::Table::Table(std::string &&s, std::vector<Column<int32_t>> &&intC, std::vector<Column<std::string>> &&stringC, std::vector<Column<bool>> &&boolC, std::vector<Column<std::vector<uint8_t>>> &&byteC) {
    name = s;
    for (auto &c : intC) {
        intColumns.push_back(c);
        columnsCount++;
    }
    for (auto &c : stringC) {
        stringColumns.push_back(c);
        columnsCount++;
    }
    for (auto &c : boolC) {
        boolColumns.push_back(c);
        columnsCount++;
    }
    for (auto &c : byteC) {
        byteColumns.push_back(c);
        columnsCount++;
    }
}

template<int ArgumentsCount, typename... Types>
memdb::Line<ArgumentsCount, Types...>::Line(bool b, std::tuple<std::pair<std::string, Types>...> &t) {
    hasNames = b;
    values = t;
}

template<int ArgumentsCount, typename... Types>
void memdb::Table::insert(Line<ArgumentsCount, Types...> &&line) {
    if (line.count != columnsCount) {
        throw ExecutionException("an attempt to insert an inappropriate number of values\n");
    }

    if (line.hasNames) {
        size_t size;
        for (auto &p : line.values) {
            for (auto &c : this->intColumns) {
                if (c.name == p.first) {
                    if (c.attributes.autoincrement) {
                        if (c.vector.empty()) {
                            c.vector.push_back(0);
                        } else {
                            c.vector.push_back(c.vector.back() + 1);
                        }
                    } else {
                        c.vector.push_back(p.second.value);
                    }
                    size = c.vector.size();
                    break;
                }
            }

            for (auto &c : this->stringColumns) {
                if (c.name == p.first) {
                    c.vector.push_back(p.second.value);
                    size = c.vector.size();
                    break;
                }
            }
            for (auto &c : this->boolColumns) {
                if (c.name == p.first) {
                    c.vector.push_back(p.second.value);
                    size = c.vector.size();
                    break;
                }
            }
            for (auto &c : this->byteColumns) {
                if (c.name == p.first) {
                    c.vector.push_back(p.second.value);
                    size = c.vector.size();
                    break;
                }
            }
        }

        for (auto &c : this->intColumns) {
            if (c.vector.size() < size) {
                if (c.attributes.autoincrement) {
                    if (c.vector.empty()) {
                        c.vector.push_back(0);
                    } else {
                        c.vector.push_back(c.vector.back() + 1);
                    }
                } else if (c.hasDefoltValue) {
                    c.vector.push_back(c.value);
                } else {
                    throw ExecutionException("Column " + c.name + " has not default value\n");
                }
            }
        }
        for (auto &c : this->stringColumns) {
            if (c.vector.size() < size) {
                if (c.hasDefoltValue) {
                    c.vector.push_back(c.value);
                } else {
                    throw ExecutionException("Column " + c.name + " has not default value\n");
                }
            }
        }
        for (auto &c : this->boolColumns) {
            if (c.vector.size() < size) {
                if (c.hasDefoltValue) {
                    c.vector.push_back(c.value);
                } else {
                    throw ExecutionException("Column " + c.name + " has not default value\n");
                }
            }
        }
        for (auto &c : this->byteColumns) {
            if (c.vector.size() < size) {
                if (c.hasDefoltValue) {
                    c.vector.push_back(c.value);
                } else {
                    throw ExecutionException("Column " + c.name + " has not default value\n");
                }
            }
        }
    } else {
        for (int i = 0; i < line.values.size(); ++i) {
            for (auto &c : this->intColumns) {
                if (c.number == i) {
                    if (line.values[i].second.defaultValue) {
                        if (c.attributes.autoincrement) {
                            if (c.vector.empty()) {
                                c.vector.push_back(0);
                            } else {
                                c.vector.push_back(c.vector.back() + 1);
                            }
                        } else if (c.hasDefoltValue) {
                            c.vector.push_back(c.value);
                        } else {
                            throw ExecutionException("Column " + c.name + " has not default value\n");
                        }
                    } else {
                        c.vector.push_back(line.values[i].second.value);
                    }
                }
            }
            for (auto &c : this->stringColumns) {
                if (c.number == i) {
                    if (line.values[i].second.defaultValue) {
                        if (c.hasDefoltValue) {
                            c.vector.push_back(c.value);
                        } else {
                            throw ExecutionException("Column " + c.name + " has not default value\n");
                        }
                    } else {
                        c.vector.push_back(line.values[i].second.value);
                    }
                }
            }
            for (auto &c : this->boolColumns) {
                if (c.number == i) {
                    if (line.values[i].second.defaultValue) {
                        if (c.hasDefoltValue) {
                            c.vector.push_back(c.value);
                        } else {
                            throw ExecutionException("Column " + c.name + " has not default value\n");
                        }
                    } else {
                        c.vector.push_back(line.values[i].second.value);
                    }
                }
            }
            for (auto &c : this->byteColumns) {
                if (c.number == i) {
                    if (line.values[i].second.defaultValue) {
                        if (c.hasDefoltValue) {
                            c.vector.push_back(c.value);
                        } else {
                            throw ExecutionException("Column " + c.name + " has not default value\n");
                        }
                    } else {
                        c.vector.push_back(line.values[i].second.value);
                    }
                }
            }
        }
    }
}