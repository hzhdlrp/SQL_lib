#pragma once
#include "memdb.hpp"
#include "memory"
#include <variant>


class Query {
public:
    virtual std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) = 0;
};

class Create : public Query {
    memdb::Table table;
public:
    Create(memdb::Table &t) : table(t) {}

    std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) override {
        db.addNewTable(std::make_shared<memdb::Table>(std::move(table)));
        return std::monostate{};
    }
};

class Insert : public Query {

    std::variant<memdb::Table, std::monostate> execute(memdb::Database &db) override {

    }
};

void execute(memdb::Database &db, Query &&q) {
    q.execute(db);
}

class Parser {

};