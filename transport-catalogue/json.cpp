#include "json.h"
#include <iostream>
#include <sstream>
#include <math.h>
using namespace std;

namespace json {

    namespace {
        Node LoadNode(istream& input);

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char]
            {
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
            }
            else {
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
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }



        // Считывает содержимое строкового литерала JSON-документа
        Node LoadString(std::istream& input)
        {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end)
                {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
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
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node{ move(s) };
        }

        Node LoadBool(std::istream& input)
        {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end)
                {
                    break;
                }
                const char ch = *it;
                if (ch == ',')
                {
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("Bool parsing error");
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
                }
                else if (ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
                    it++;
                }
                else if (ch == '}' || ch == ']') {
                    break;
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            if (s.find("true"s) != s.npos)
            {
                return Node(true);
            }
            else if (s.find("false"s) != s.npos)
            {
                return Node(false);
            }
            throw ParsingError("Bool parsing error");
        }

        Node LoadNull(std::istream& input)
        {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end)
                {
                    break;
                }
                const char ch = *it;
                if (ch == ',')
                {
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("Null parsing error");
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
                }
                else if (ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
                    it++;
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            if (s == "null"s)
            {
                return Node(nullptr);
            }
            throw ParsingError("Null parsing error");
        }

        Node LoadArray(std::istream& input)
        {
            Array array;
            for (char ch = 0; input >> ch && ch != ']';)
            {
                if (ch != ',')
                    input.putback(ch);

                array.push_back(LoadNode(input));

                if (input >> ch)
                {
                    if (ch == ']')
                    {
                        break;
                    }
                    else if (ch != ',')
                    {
                        throw ParsingError("Array parsing error, no',' "); // Элементы массива не разделены запятой
                    }
                }
                else
                {
                    throw ParsingError("Array parsing error, no symbol"); // Прочитав очередной элемент массива не встрелили никаких символов
                }
            }

            if (input)
            {
                return Node{ std::move(array) };
            }
            else
            {
                throw ParsingError("Array parsing error, no close skobka"); // Вышли из цикла, не найдя закрывающуюся скобку
            }
        }

        Node LoadDict(std::istream& input)
        {
            Dict dict;
            for (char ch; input >> ch && ch != '}';)
            {
                if (ch == '"')
                {
                    std::string dict_key = LoadString(input).AsString();
                    if (dict.count(dict_key) > 0)
                    {
                        throw ParsingError("Array parsing error"); // Дублируется ключ
                    }
                    else if (input >> ch && ch != ':')
                    {
                        throw ParsingError("Array parsing error"); // Ключ без двоеточия
                    }

                    dict[move(dict_key)] = LoadNode(input);
                }
                else if (ch != ',')
                {
                    throw ParsingError("Array parsing error"); // Элементы словаря не разделены запятой
                }
            }

            if (input)
            {
                return Node{ std::move(dict) };
            }
            else
            {
                throw ParsingError("Array parsing error"); // Вышли из цикла, не найдя закрывающуюся скобку
            }
        }

        Node LoadNode(istream& input)
        {
            char ch;
            input >> ch;
            if (ch == '[')
            {
                return LoadArray(input);
            }
            else if (ch == '{')
            {
                return LoadDict(input);
            }
            else if (ch == '"')
            {
                return LoadString(input);
            }
            else if (ch == 't' || ch == 'f')
            {
                input.putback(ch);
                return LoadBool(input);
            }
            else if (ch == 'n')
            {
                input.putback(ch);
                return LoadNull(input);
            }
            else {
                input.putback(ch);
                auto num = LoadNumber(input);
                if (std::holds_alternative<int>(num))
                {
                    return Node{ std::get<int>(num) };
                }
                else
                {
                    return Node{ std::get<double>(num) };
                }

            }
        }

    }  // namespace


    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Node& doc, std::ostream& out) {
        // Выводим в поток strm
        visit(OstreamSolutionPrinter{ out }, doc.GetValue());
        // Реализуйте функцию самостоятельно
    }

    void Print(const Document& doc, std::ostream& output) {
        // Выводим в поток strm
        // Реализуйте функцию самостоятельно

        visit(OstreamSolutionPrinter{ output }, doc.GetRoot().GetValue());
    }

    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(GetValue());
    }

    bool Node::IsDouble() const
    {
        if (std::holds_alternative<int>(GetValue())) {
            return std::holds_alternative<int>(GetValue());
        }
        else {
            return std::holds_alternative<double>(GetValue());
        }
        //return  std::holds_alternative<double>(GetValue()) || std::holds_alternative<int>(GetValue());
    }

    bool Node::IsPureDouble() const
    {
        return  std::holds_alternative<double>(GetValue());
    }

    bool Node::IsBool() const
    {
        return  std::holds_alternative<bool>(GetValue());
    }

    bool Node::IsString() const
    {
        return  std::holds_alternative<string>(GetValue());
    }

    bool Node::IsNull() const
    {
        return  std::holds_alternative<std::nullptr_t>(GetValue());
    }

    bool Node::IsArray() const
    {
        return  std::holds_alternative<Array>(GetValue());
    }

    bool Node::IsMap() const
    {
        return  std::holds_alternative<Dict>(GetValue());
    }


    const std::string& Node::AsString() const
    {
        if (std::holds_alternative<string>(GetValue())) {
            return get<string>(GetValue());
        }
        else {
            throw std::logic_error("AsString");
        }
    }

    int Node::AsInt() const
    {
        if (std::holds_alternative<int>(GetValue())) {
            return get<int>(GetValue());
        }
        else {
            throw std::logic_error("AsInt");
        }
    }
    double Node::AsDouble() const
    {
        if (std::holds_alternative<int>(GetValue())) {
            return this->AsInt();
        }
        else if (std::holds_alternative<double>(GetValue())) {
            return get<double>(GetValue());
        }
        else {
            throw std::logic_error("AsDouble");
        }
    }
    bool Node::AsBool() const
    {
        if (std::holds_alternative<bool>(GetValue())) {
            return get<bool>(GetValue());
        }
        else {
            throw std::logic_error("AsBool");
        }
    }
    const Array& Node::AsArray() const
    {
        if (std::holds_alternative<Array>(GetValue())) {
            return get<Array>(GetValue());
        }
        else {
            throw std::logic_error("AsArray");
        }
    }
    const Dict& Node::AsMap() const
    {
        if (std::holds_alternative<Dict>(GetValue())) {
            return get<Dict>(GetValue());
        }
        else {
            throw std::logic_error("AsMap");
        }
    }

    /*
     bool operator!=(const Node& lhs, const Node& rhs)
     {
         return !(lhs==rhs);
     }

     bool operator==(const Document& lhs, const Document& rhs) {
         return lhs.GetRoot() == rhs.GetRoot();
     }
     bool operator!=(const Document& lhs, const Document& rhs) {
         return !(lhs == rhs);
     }*/
}  // namespace json

