#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <ostream>
#include <sstream>

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
    using Value = std::variant <std::nullptr_t, Dict, Array, bool, std::string, double, int>;

    class Node final : Value {
    public:
        /* Реализуйте Node, используя std::variant */
        using variant::variant;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;


        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const
        {
            return *this;
        }
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    struct OstreamSolutionPrinter {
        std::ostream& out;
        void operator()(const std::nullptr_t) const {
            out << "null";
        }
        void operator()(const Array& array) const
        {
            bool separator_flag = false;
            out << '[';
            for (const auto& value : array)
            {
                if (separator_flag)
                {
                    out << ", ";
                }
                std::visit(OstreamSolutionPrinter{ out }, value.GetValue());
                separator_flag = true;
            }
            out << ']';
        }

        void operator()(const Dict& dict) const
        {
            bool separator_flag = false;
            out << '{';
            for (const auto& [dict_key, value] : dict)
            {
                if (separator_flag)
                {
                    out << ", ";
                }
                std::visit(OstreamSolutionPrinter{ out }, Node{ dict_key }.GetValue());
                out << ':';
                std::visit(OstreamSolutionPrinter{ out }, Node{ value }.GetValue());
                separator_flag = true;
            }
            out << '}';
        }
        void operator()(const bool data) const {
            if (data) {
                out << "true";
            }
            else {
                out << "false";
            }
        }
        void operator()(const std::string& str) const {
            using namespace std::literals;
            out << "\""sv;
            for (const char ch : str)
            {
                switch (ch)
                {
                case '\t':
                    out << "\t"sv;
                    break;
                case '\r':
                    out << "\\r"sv;
                    break;
                case '\n':
                    out << "\\n"sv;
                    break;
                case '"':
                    out << '\\';
                    out << ch;
                    break;
                case '\\':
                    out << '\\';
                    out << ch;
                    break;
                default:
                    out << ch;
                    break;
                }
            }
            out << "\""sv;
        }
        void operator()(const double data) const {
            out << data;
        }
        void operator()(const int data) const {
            out << data;
        }
    };
    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);
    void Print(const Node& doc, std::ostream& out);


    inline bool operator == (const json::Node& lhs, const json::Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    inline bool operator != (const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }
    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }
    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    inline Document LoadJSON(const std::string& s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }
}  // namespace json