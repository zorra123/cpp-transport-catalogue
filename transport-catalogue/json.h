#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <ostream>
#include <sstream>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final
    : private std::variant< std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;

    Node(Value& val);
    bool IsInt() const;
    int AsInt() const;
    bool IsPureDouble() const;
    bool IsDouble() const;
    double AsDouble() const;
    bool IsBool() const;
    bool AsBool() const;
    bool IsNull() const;
    bool IsArray() const;
    const Array& AsArray() const;
    bool IsString() const;
    const std::string& AsString() const;
    bool IsDict() const;
    const Dict& AsDict() const;
    bool operator==(const Node& rhs) const;
    void AddToDict(std::string key, Node& node);
    void AddToArray(Node& node);

    const Value& GetValue() const {
        return *this;
    }

     Value& GetNonConstValue()  {
        return *this;
    }
};

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    explicit Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& GetRoot() const {
        return root_;
    }

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

inline Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

}  // namespace json