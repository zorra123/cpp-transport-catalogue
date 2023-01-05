#pragma once
#include "json.h"
#include <vector>
#include <stack>


namespace json {

    class Builder {

    private:
        class Context;
        class KeyItem;
        class KeyValueItem;
        class DictItem;
        class ArrayItem;

        class Context {
        public:
            Context(Builder& builder) : builder_{ builder } {}
        protected:
            KeyItem Key(std::string key);
            DictItem StartDict();
            Builder& EndDict();
            ArrayItem StartArray();
            Builder& EndArray();
            Builder& builder_;
        };

        class KeyValueItem final : public Context {
        public:
            using Context::Context;
            using Context::Key;
            using Context::EndDict;
        };

        class KeyItem final : public Context {
        public:
            using Context::Context;

            template <typename T>
            KeyValueItem Value(T value) {
                builder_.Value(std::move(value));
                return KeyValueItem{ builder_ };
            }

            //KeyValueItem Value(json::Node::Value value);
            using Context::StartDict;
            using Context::StartArray;
        };

        class DictItem final : public Context {
        public:
            using Context::Context;
            using Context::Key;
            using Context::EndDict;
        };

        class ArrayItem final : public Context {
        public:
            using Context::Context;

            template <typename T>
            ArrayItem Value(T value) {
                builder_.Value(std::move(value));
                return ArrayItem{ builder_ };
            }

            //ArrayItem Value(json::Node::Value value);
            using Context::StartDict;
            using Context::StartArray;
            using Context::EndArray;
        };

    public:
        Builder() {
            state.push(CREATE);
        }
        KeyItem Key(std::string);

        //хочу принимать как value, так и готовые Node
        template <typename T>
        Builder& Value(T val) {
            if (state.top() == Statements::KEY) {
                state.pop();
            }
            else if (!(state.top() == Statements::CREATE || state.top() == Statements::STARTARRAY)) {
                throw std::logic_error("Problems with insert value");
            }
            else if (state.top() == Statements::CREATE) {
                state.push(Statements::VALUE);
            }
            nodes_stack_.push_back(new Node(val));
            return *this;
        }
        //Builder& Value(Node::Value val);

        DictItem StartDict();
        Builder& EndDict();

        ArrayItem StartArray();
        Builder& EndArray();
        Node Build();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        enum Statements {
            CREATE,
            STARTDICT,
            STARTARRAY,
            ENDDICT,
            ENDARRAY,
            KEY,
            VALUE
        };
        std::stack< Statements> state;
    };

}