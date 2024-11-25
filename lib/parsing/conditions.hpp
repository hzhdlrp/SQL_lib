#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>

namespace conditions {
    enum Type {
        INT,
        BOOL,
        STRING,
        BYTE,
        VARIABLE,
        BINOP,
        UNOP
    };

    bool isNumber(std::string s) {
        for (char &c : s) {
            if (c < '0' || c > '9') return false;
        }
        return true;
    }

    bool isWord(std::string &s) {
        if (!s.empty() && (s[0] >= 'a' && s[0] <= 'z' || s[0] >= 'A' && s[0] <= 'Z'))
            return true;
        return false;
    }

    bool isGood(const char input) {
        if (input >= '0' && input <= '9' ||
            input >= 'a' && input <= 'z' ||
            input >= 'A' && input <= 'Z' ||
            input == '\"' || input == '_' || input == '-') return true;
        return false;
    }

    struct Expression {
        Expression(std::string token) : token(token) {}
        Expression(std::string token, Expression a) : token(token), args{a} {}
        Expression(std::string token, Expression a, Expression b) : token(token), args{a, b} {}

        std::string token;
        std::vector<Expression> args;
    };

    int get_priority(const std::string& token) {
        if (token == "||") return 1;
        if (token == "&&" || token == "^^") return 2;
        if (token == ">" || token == "<" || token == ">=" || token == "<=" || token == "==" || token == "!=") return 3;
        if (token == "+") return 4;
        if (token == "-") return 4;
        if (token == "*") return 5;
        if (token == "/") return 5;
        if (token == "%") return 5;
        if (token == "!") return 6;
        return 0; // Возвращаем 0 если токен - это не бинарная операция (например ")")
    }

    class Parser {

        // !!!_ALERT_!!!
        // частично скопипастил отсюда
        // https://ru.stackoverflow.com/questions/23842
        // но внес очень значительные изменения в логику работы

    public:
        explicit Parser(const char* input) : input(input) {} // Конструктор, принимает строку с выражением
        Expression parse() {
            return parse_binary_expression(0);
        }

        std::set<std::string> getVariablesNames() {
            make_tokens_sequence();
            makeVariablesSet();
            return variables;
        }

        void setVariables(std::map<std::string, std::string> &map) {
            if (map.size() != getVariablesNum()) {
                throw std::runtime_error("invalid arguments count\n");
            }

            for (auto &t : tokens_sequence) {
                if (map.contains(t)) {
                    t = map[t];
                }
            }
        }

        std::variant<int32_t, bool, std::string> eval(const Expression& e) {
            // на самом деле байтовая последовательность в таблице тоже хранится как строка

            switch (e.args.size()) {
                case 2: {
                    auto a_v = eval(e.args[0]);
                    auto b_v = eval(e.args[1]);
                    if (a_v.index() != b_v.index())
                        throw std::runtime_error("types mismatching\n");
                    switch (a_v.index()) {
                        case INT :{
                            auto &a = std::get<INT>(a_v);
                            auto &b = std::get<INT>(b_v);

                            if (e.token == "+") return a + b;
                            if (e.token == "-") return a - b;
                            if (e.token == "*") return a * b;
                            if (e.token == "/") return a / b;
                            if (e.token == "%") return a % b;
                            if (e.token == ">") return a > b;
                            if (e.token == "<") return a < b;
                            if (e.token == ">=") return a >= b;
                            if (e.token == "<=") return a <= b;
                            if (e.token == "==") return a == b;
                            if (e.token == "!=") return a != b;
                            throw std::runtime_error("Unknown binary operator " + e.token + " for int operands\n");
                            break;
                        }
                        case BOOL :{
                            auto &a = std::get<BOOL>(a_v);
                            auto &b = std::get<BOOL>(b_v);

                            if (e.token == ">") {
                                if (a && !b) return true;
                                return false;
                            }
                            if (e.token == "<") {
                                if (!a && b) return true;
                                return false;
                            }
                            if (e.token == ">=") {
                                if (!a && b) return false;
                                return true;
                            }
                            if (e.token == "<=") {
                                if (a && !b) return false;
                                return true;
                            }
                            if (e.token == "==") return a == b;
                            if (e.token == "!=") return a != b;
                            if (e.token == "&&") return a && b;
                            if (e.token == "||") return a || b;
                            if (e.token == "^^") return (a == b ? false : true);
                            throw std::runtime_error("Unknown binary operator " + e.token + " for bool operands\n");
                            break;
                        }
                        case STRING: {
                            std::string &a = std::get<STRING>(a_v);
                            std::string &b = std::get<STRING>(b_v);

                            if (a[0] == '\"') {
                                if (b[0] == '\"') {

                                    size_t start {a.find("\"")};
                                    while (start != std::string::npos)
                                    {
                                        a.erase(start, 1);
                                        start = a.find("\"", start + 1);
                                    }
                                    size_t bstart {b.find("\"")};
                                    while (bstart != std::string::npos)
                                    {
                                        b.erase(bstart, 1);
                                        bstart = b.find("\"", bstart + 1);
                                    }
//                                    a.erase(std::remove(a.begin(), a.end(), '\"'), a.end());
//                                    b.erase(std::remove(b.begin(), b.end(), '\"'), b.end());

                                    if (e.token == "+") {
                                        return "\"" + a + b + "\"";
                                    }
                                    throw std::runtime_error("Unknown binary operator " + e.token + " for string operands\n");
                                    break;

                                }
                                throw std::runtime_error("types mismatching\n");
                            } else {
                                throw std::runtime_error("Unknown binary operator " + e.token + " for (maybe byte type idk) operands\n");
                            }
                            break;
                        }
                            break;
                    }

                }

                case 1: {
                    auto a_v = eval(e.args[0]);
                    switch (a_v.index()) {
                        case INT: {
                            auto &a = std::get<INT>(a_v);
                            if (e.token == "+") return +a;
                            if (e.token == "-") return -a;
                            throw std::runtime_error("Unknown unary operator " + e.token + " for int operand\n");
                            break;
                        }
                        case BOOL: {
                            auto &a = std::get<INT>(a_v);
                            if (e.token == "!") return !a;
                            throw std::runtime_error("Unknown unary operator " + e.token + " for bool operand\n");
                            break;
                        }
                        case STRING: {
                            auto &a = std::get<STRING>(a_v);
                            if (e.token == "|") return int32_t(a.size()) - 2;
                            throw std::runtime_error("Unknown unary operator " + e.token + " for string operand\n");
                            break;
                        }
                    }
                }

                case 0: {
                    if (e.token == "true") return true;
                    if (e.token == "false") return false;
                    if (isNumber(e.token)) return std::stoi(e.token);
                    return e.token;
                }
            }

            throw std::runtime_error("Unknown expression type");
        }
    private:
        int index = 0;

        std::set<std::string> variables;

        void makeVariablesSet() {
            for (auto &t : tokens_sequence) {
                if (isWord(t)) {
                    variables.insert(t);
                }
            }
        }

        int getVariablesNum() {
            return variables.size();
        }

        std::string parse_token() {
            // Пропускаем пробелы перед токеном.
            while (std::isspace(*input)) ++input;

            if (isGood(*input)) {
                std::string token;
                while (isGood(*input)) token.push_back(*input++);
                return token;
            }

            // Список всех известных токенов - операции и скобки.
            static const std::string tokens[] =
                    { "+", "-", "*", "/", "%",  "|", "&&", "||", "^^", "(", ")", "==", "!=", ">", "<", ">=", "<=" };
            bool absIsOpened = false;
            for (auto& t : tokens) {
                if (std::strncmp(input, t.c_str(), t.size()) == 0) {
                    input += t.size();
                    if (t == "|") {
                        if (absIsOpened) {
                            absIsOpened = false;
                            return std::string{"|c"};
                        } else {
                            absIsOpened = true;
                            return std::string{"|o"};
                        }
                    }
                    return t;
                }
            }

            return ""; // Какой-то неизвестный токен, или символ '\0' - конец строки.
        }

        void make_tokens_sequence() {
            std::string token = parse_token();
            while (!token.empty()) {
                tokens_sequence.push_back(token);
                token = parse_token();
            }
        }

        const char* input;
        std::vector<std::string> tokens_sequence;

        Expression parse_binary_expression(int min_priority) {
            auto left_expr = parse_simple_expression();

            for (;;) {
                if (index >= tokens_sequence.size())
                    throw std::runtime_error("Invalid input");
                auto op = tokens_sequence[index]; // Пробуем парсить бинарную операцию.
                ++index;

                auto priority = get_priority(op);
                // Выходим из цикла если ее приоритет слишком низок (или это не бинарная операция).
                if (priority <= min_priority) {
                    input -= op.size(); // Отдаем токен обратно,
                    return left_expr; // возвращаем выражение слева.
                }

                auto right_expr = parse_binary_expression(priority);
                left_expr = Expression(op, left_expr, right_expr); // Обновляем выражение слева.
            } // Повторяем цикл: парсинг операции, и проверка ее приоритета.
        }

        Expression parse_simple_expression() {
            if (index >= tokens_sequence.size()) {
                throw std::runtime_error("Invalid input");
            }
            auto token = tokens_sequence[index];
            ++index;

            if (isGood(token[0]))
                return Expression(token);

            if (token == "(") {
                auto result = parse();
                if (index >= tokens_sequence.size() || tokens_sequence[index] != ")") throw std::runtime_error("Expected ')'");
                ++index;
                return result; // Если это скобки, парсим и возвращаем выражение в скобках
            }

            if (token == "|o") {
                auto result = Expression("|", parse());
                if (index >= tokens_sequence.size() || tokens_sequence[index] != "|c") throw std::runtime_error("Expected '|'");
                ++index;
                return result;
            }

            // Иначе, это унарная операция
            auto arg = parse_simple_expression(); // Парсим ее аргумент.
            return Expression(token, arg);
        }


        enum {
            INT = 0,
            BOOL = 1,
            STRING = 2
        };

    };



}
