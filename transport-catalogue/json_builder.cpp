#include "json_builder.h"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace json {
    Context::Context(Builder* builder) :
        builder_(builder)
    {}

    Builder* Context::GetBuilder() {
        return builder_;
    }

    KeyContext GenericContext::Key(std::string key) {
        return builder_->Key(std::move(key));
    }

    GenericContext GenericContext::Value(Node::Value val) {
        return builder_->Value(std::move(val));
    }

    DictItemContext GenericContext::StartDict() {
        return builder_->StartDict();
    }

    GenericContext GenericContext::EndDict() {
        return builder_->EndDict();
    }

    ArrayItemContext GenericContext::StartArray() {
        return builder_->StartArray();
    }

    GenericContext GenericContext::EndArray() {
        return builder_->EndArray();
    }

    Node GenericContext::Build() {
        return builder_->Build();
    }

    DictItemContext KeyContext::Value(Node::Value val) {
        return builder_->Value(std::move(val));
    }

    DictItemContext KeyContext::StartDict() {
        return builder_->StartDict();
    }

    ArrayItemContext KeyContext::StartArray() {
        return builder_->StartArray();
    }

    DictItemContext::DictItemContext(Context con) :
        Context(con.GetBuilder())
    {}

    KeyContext DictItemContext::Key(std::string key) {
        return builder_->Key(std::move(key));
    }

    GenericContext DictItemContext::EndDict() {
        return builder_->EndDict();
    }

    ArrayItemContext::ArrayItemContext(Context con) :
        Context(con.GetBuilder())
    {}

    ArrayItemContext ArrayItemContext::Value(Node::Value val) {
        return builder_->Value(std::move(val));
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_->StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_->StartArray();
    }

    GenericContext ArrayItemContext::EndArray() {
        return builder_->EndArray();
    }

    KeyContext Builder::Key(std::string key) {
        if (!started_) {
            throw std::logic_error("Not in Dict context.");
        }
        if (nodes_stack_.empty()) {
            throw std::logic_error("Object complete.");
        }
        if (!(nodes_stack_.back()->IsDict())) {
            throw std::logic_error("Not in Dict context.");
        }
        if (key_.has_value()) {
            throw std::logic_error("Previous key not resolved into value.");
        }
        key_ = key;
        return {this};
    }

    GenericContext Builder::Value(Node::Value val) {
        if (!started_) {
            started_ = true;
            std::visit([this](auto&& arg){this->root_ = arg;}, val);
            return {this};
        }
        if (nodes_stack_.empty()) {
            throw std::logic_error("Object complete.");
        }
        if (nodes_stack_.back()->IsDict()) {
            if (!key_.has_value()) {
                throw std::logic_error("Key not set.");
            }
            std::visit([this](auto&& arg){std::get<Dict>(this->nodes_stack_.back()->GetValue())[key_.value()] = arg;}, val);
            key_.reset();
        } else if (nodes_stack_.back()->IsArray()) {
            std::visit([this](auto&& arg){std::get<Array>(this->nodes_stack_.back()->GetValue()).push_back(arg);}, val);
            key_.reset();
        } else {
            throw std::logic_error("Wrong context for Value().");
        }
        return {this};
    }

    DictItemContext Builder::StartDict() {
        Dict dict;
        Node* node_ptr;
        if (!started_) {
            started_ = true;
            root_ = dict;
            node_ptr = &root_;
        } else if (nodes_stack_.empty()) {
            throw std::logic_error("Object complete.");
        } else if (nodes_stack_.back()->IsDict()) {
            if (!key_.has_value()) {
                throw std::logic_error("Key not set.");
            }
            std::get<Dict>(this->nodes_stack_.back()->GetValue())[key_.value()] = dict;
            node_ptr = &(std::get<Dict>(this->nodes_stack_.back()->GetValue()).at(key_.value()));
            key_.reset();
        } else if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(this->nodes_stack_.back()->GetValue()).push_back(dict);
            node_ptr = &(std::get<Array>(this->nodes_stack_.back()->GetValue()).back());
        } else {
            throw std::logic_error("Wrong context for StartDict().");
        }
        nodes_stack_.push_back(node_ptr);
        return {this};
    }

    GenericContext Builder::EndDict() {
        if (!started_) {
            throw std::logic_error("Not in Dict context.");
        }
        if (nodes_stack_.empty()) {
            throw std::logic_error("Object complete.");
        }
        if (!(nodes_stack_.back()->IsDict())) {
            throw std::logic_error("Not in Dict context.");
        }
        if (key_.has_value()) {
            throw std::logic_error("Key not resolved.");
        }
        nodes_stack_.pop_back();
        return {this};
    }

    ArrayItemContext Builder::StartArray() {
        Array array;
        Node* node_ptr;
        if (!started_) {
            started_ = true;
            root_ = array;
            node_ptr = &root_;
        } else if (nodes_stack_.empty()) {
            throw std::logic_error("Object complete.");
        } else if (nodes_stack_.back()->IsDict()) {
            if (!key_.has_value()) {
                throw std::logic_error("Key not set.");
            }
            std::get<Dict>(this->nodes_stack_.back()->GetValue())[key_.value()] = array;
            node_ptr = &(std::get<Dict>(this->nodes_stack_.back()->GetValue()).at(key_.value()));
            key_.reset();
        } else if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(this->nodes_stack_.back()->GetValue()).push_back(array);
            node_ptr = &(std::get<Array>(this->nodes_stack_.back()->GetValue()).back());
        } else {
            throw std::logic_error("Wrong context for StartDict().");
        }
        nodes_stack_.push_back(node_ptr);
        return {this};
    }

    GenericContext Builder::EndArray() {
        if (!started_) {
            throw std::logic_error("Not in Array context.");
        }
        if (nodes_stack_.empty()) {
            throw std::logic_error("Object complete.");
        }
        if (!(nodes_stack_.back()->IsArray())) {
            throw std::logic_error("Not in Array context.");
        }
        nodes_stack_.pop_back();
        return {this};
    }

    Node Builder::Build() {
        if (!started_) {
            throw std::logic_error("Document has not been built at all.");
        }
        if (nodes_stack_.size() > 0) {
            throw std::logic_error("Document has not been completed.");
        }
        return root_;
    }
}
