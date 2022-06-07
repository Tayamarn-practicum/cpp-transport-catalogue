#include "json.h"

#include <stdexcept>

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;

        Number LoadNumber_(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString_(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            char c;
            for (;input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("Parsing error"s);
            }

            return Node(move(result));
        }

        Node LoadNumber(istream& input) {
            Number num = LoadNumber_(input);
            if (std::holds_alternative<double>(num)) {
                return Node(std::get<double>(num));
            }
            return Node(std::get<int>(num));
        }

        Node LoadString(istream& input) {
            std::string line = LoadString_(input);
            return Node(move(line));
        }

        Node LoadDict(istream& input) {
            Dict result;

            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            if (c != '}') {
                throw ParsingError("Parsing error"s);
            }

            return Node(move(result));
        }

        Node LoadNull(istream& input) {
            char c1, c2, c3;
            input >> c1 >> c2 >> c3;
            if (c1 != 'u' || c2 != 'l' || c3 != 'l') {
                throw ParsingError("");
            }
            return Node(nullptr);
        }

        Node LoadTrue(istream& input) {
            char c1, c2, c3;
            input >> c1 >> c2 >> c3;
            if (c1 != 'r' || c2 != 'u' || c3 != 'e') {
                throw ParsingError("");
            }
            return Node(true);
        }

        Node LoadFalse(istream& input) {
            char c1, c2, c3, c4;
            input >> c1 >> c2 >> c3 >> c4;
            if (c1 != 'a' || c2 != 'l' || c3 != 's' || c4 != 'e') {
                throw ParsingError("");
            }
            return Node(false);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 'n') {
                return LoadNull(input);
            } else if (c == 't') {
                return LoadTrue(input);
            } else if (c == 'f') {
                return LoadFalse(input);
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node(std::nullptr_t val) :
        value_(val)
    {};
    Node::Node(Array val) :
        value_(val)
    {};
    Node::Node(Dict val) :
        value_(val)
    {};
    Node::Node(bool val) :
        value_(val)
    {};
    Node::Node(int val) :
        value_(val)
    {};
    Node::Node(double val) :
        value_(val)
    {};
    Node::Node(std::string val) :
        value_(val)
    {};

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("");
        }
        return get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("");
        }
        return get<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("");
        }
        return get<int>(value_);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("");
        }
        return get<std::string>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("");
        }
        return get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("");
        }
        if (IsInt()) {
            return static_cast<double>(get<int>(value_));
        }
        return get<double>(value_);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool operator==(const Node& lhs, const Node& rhs)
    {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() != rhs.GetValue();
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() != rhs.GetRoot();
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void PrintValue(const int value, std::ostream& out) {
        out << value;
    }

    void PrintValue(const double value, std::ostream& out) {
        out << value;
    }

    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null";
    }

    void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    void PrintValue(const std::string value, std::ostream& out) {
        std::string res = value;
        ReplaceAll(res, "\\", "\\\\");
        ReplaceAll(res, "\"", "\\\"");
        ReplaceAll(res, "\n", "\\n");
        ReplaceAll(res, "\r", "\\r");
        out << "\"" << res << "\"";
    }

    void PrintValue(const bool value, std::ostream& out) {
        out << std::boolalpha << value;
    }

    void PrintValue(const Array value, std::ostream& out) {
        out << "[";
        bool first = true;
        for (Node node : value) {
            if (first) {
                first = false;
            } else {
                out << ",";
            }
            PrintNode(node, out);
        }
        out << "]";
    }

    void PrintValue(const Dict value, std::ostream& out) {
        out << "{";
        bool first = true;
        for (auto [key, node] : value) {
            if (first) {
                first = false;
            } else {
                out << ",";
            }
            out << "\"" << key << "\":";
            PrintNode(node, out);
        }
        out << "}";
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value){ PrintValue(value, out); },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json
