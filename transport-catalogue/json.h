#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(std::nullptr_t val) :
            value_(val)
        {};
        Node(Array val) :
            value_(val)
        {};
        Node(Dict val) :
            value_(val)
        {};
        Node(bool val) :
            value_(val)
        {};
        Node(int val) :
            value_(val)
        {};
        Node(double val) :
            value_(val)
        {};
        Node(std::string val) :
            value_(val)
        {};

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        const Value& GetValue() const;

    private:
        Value value_ = nullptr;
    };

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    void PrintValue(const int value, std::ostream& out);
    void PrintValue(const double value, std::ostream& out);
    void PrintValue(std::nullptr_t, std::ostream& out);
    void PrintValue(const std::string value, std::ostream& out);
    void PrintValue(const bool value, std::ostream& out);
    void PrintValue(const Array value, std::ostream& out);
    void PrintValue(const Dict value, std::ostream& out);
    void PrintNode(const Node& node, std::ostream& out);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
