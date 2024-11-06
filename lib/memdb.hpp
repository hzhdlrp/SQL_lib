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
    private:
        void foo() override {};
        bool hasDefoltValue = false;
        int len; // for strings and bytes
    };


    template<int ArgumentsCount, typename... Types>
    struct Line {
        Line(bool, std::tuple<std::pair<std::string, Types>...> &);
        std::tuple<std::pair<std::string, Types>...> values;
        bool hasNames = false;
        int count = ArgumentsCount;
    };

    class Table {
        std::vector<Column<int32_t>> intColumns;
        std::vector<Column<bool>> boolColumns;
        std::vector<Column<std::string>> stringColumns;
        std::vector<Column<std::vector<uint8_t>>> byteColumns;
        int columnsCount = 0;
    public:
        std::string name;

        template<typename... Types>
        Table(std::string &&s, std::tuple<Column<Types>...> &&columns);

        template<int ArgumentsCount, typename... Types>
        void insert(Line<ArgumentsCount, Types...> &&);

    };


    class Database {
        std::vector<Table> tables;
        std::vector<const std::type_info*> types;

    public:
        void addNewTable(const std::shared_ptr<TableBase>&, const std::type_info *);
        void insertToTable(std::string &values, std::string &name);
    };

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