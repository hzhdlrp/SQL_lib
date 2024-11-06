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
    private:
        virtual void foo() = 0;
    };

    template <typename T>
    struct Column : public ColumnInterface {
        T value;
        Column(std::string &&);
        Column(std::string &&, T &&);
        Column(std::string &&, ColumnAttributes &&attr, T &&);
        Column(std::string &&, ColumnAttributes &&);
    private:
        void foo() override {};
    };

    template<typename... Types>
    class Line {
        std::tuple<Types...> values;
    };

    class TableBase {
        virtual std::type_info getType() const = 0;
    };

    template<typename... Types>
    class Table : public TableBase{
        std::vector<std::shared_ptr<ColumnInterface>> columns;
        std::vector<std::type_info> types;
        std::vector<Line<Types...>> lines;
    public:
        std::string name;
        void insert(Line<Types...> &&);
        std::type_info getType() const override;
    };


    class Database {
        std::vector<std::shared_ptr<TableBase>> tables;
        std::vector<std::type_info> types;

    public:
        void addNewTable(const std::shared_ptr<TableBase>&, const std::type_info);

    };

}