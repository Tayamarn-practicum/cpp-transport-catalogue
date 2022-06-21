#pragma once

#include <optional>
#include <string>
#include <vector>

#include "json.h"

namespace json {
    class Builder;
    class GenericContext;
    class KeyContext;
    class DictItemContext;
    class ArrayItemContext;

    class Context {
    public:
        Context(Builder* builder);

        Builder* GetBuilder();
    protected:
        Builder* builder_;
    };

    class GenericContext : public Context {
    public:
        KeyContext Key(const std::string& key);
        GenericContext Value(Node::Value val);

        DictItemContext StartDict();
        GenericContext EndDict();

        ArrayItemContext StartArray();
        GenericContext EndArray();

        Node Build();
    };

    class KeyContext : public Context {
    public:
        DictItemContext Value(Node::Value val);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    };

    class DictItemContext : public Context {
    public:
        DictItemContext(Context con);

        KeyContext Key(const std::string& key);

        GenericContext EndDict();
    };

    class ArrayItemContext : public Context {
    public:
        ArrayItemContext(Context con);

        ArrayItemContext Value(Node::Value val);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        GenericContext EndArray();
    };

    class Builder {
    public:
        Builder() = default;

        KeyContext Key(const std::string& key);
        GenericContext Value(Node::Value val);

        DictItemContext StartDict();
        GenericContext EndDict();

        ArrayItemContext StartArray();
        GenericContext EndArray();

        Node Build();
    private:
        bool started_ = false;
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> key_;
    };
}
