#pragma once
#include "memdb.hpp"
#include "memory"
#include <variant>

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

class Parser {
    std::shared_ptr<Query> parse(std::string &&string) {
        return std::make_shared<Query>(Insert());
    }
};