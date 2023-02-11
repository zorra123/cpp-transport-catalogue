#include "json_builder.h"
using namespace json;


Builder::KeyItem Builder::Key(std::string key){
	if (state.top() != Statements::STARTDICT) {
		throw std::logic_error("Problems with insert key");
	}
	state.push(KEY);
	nodes_stack_.push_back(new Node(key));
	return KeyItem(*this);
}

Builder::DictItem json::Builder::StartDict() {
	if (!(state.top() == Statements::CREATE || state.top() == Statements::KEY || state.top() == Statements::STARTARRAY)) {
		throw std::logic_error("Problems with start dict");
	}
	nodes_stack_.push_back(new Node(json::Dict()));
	state.push(STARTDICT);
	return Builder::DictItem(*this);
}



Builder::ArrayItem json::Builder::StartArray()
{
	if (!(state.top() == Statements::CREATE || state.top() == Statements::KEY || state.top() == Statements::STARTARRAY)) {

		throw std::logic_error("Problems with start array");
	}
	nodes_stack_.push_back(new Node(json::Array()));
	state.push(STARTARRAY);
	return Builder::ArrayItem(*this);
}

Builder& json::Builder::EndDict()
{
	if (state.top() != Statements::STARTDICT) {
		throw std::logic_error("Problems with close dict");
	}
	state.pop();
	if (state.top() == Statements::KEY) {
		state.pop();
	}
	auto it_dict_to_push = std::prev(nodes_stack_.end());
	while (!((*it_dict_to_push)->IsDict() && (*it_dict_to_push)->AsDict().empty())) {
		--it_dict_to_push;
	}
	for (auto it_element = std::next(it_dict_to_push); it_element < nodes_stack_.end(); it_element += 2) {

		(**it_dict_to_push).AddToDict((*it_element)->AsString(), (**std::next(it_element)));
		delete* it_element;
		delete* std::next(it_element);
	}
	nodes_stack_.resize(std::distance(nodes_stack_.begin(), it_dict_to_push) + 1);
	return *this;
}

Builder& json::Builder::EndArray()
{
	if (state.top() != Statements::STARTARRAY) {
		throw std::logic_error("Problems with close dict");
	}
	state.pop();
	if (state.top() == Statements::KEY) {
		state.pop();
	}
	auto it_arr_to_push = std::prev(nodes_stack_.end());
	while (!((*it_arr_to_push)->IsArray() && (*it_arr_to_push)->AsArray().empty())) {
		--it_arr_to_push;
	}
	for (auto it_element = std::next(it_arr_to_push); it_element < nodes_stack_.end(); ++it_element) {

		(**it_arr_to_push).AddToArray(**it_element);
		delete* it_element;
	}
	nodes_stack_.resize(std::distance(nodes_stack_.begin(), it_arr_to_push) + 1);

	return *this;
}

Node json::Builder::Build()
{
	if (!(state.top() == Statements::CREATE || state.top() == Statements::VALUE) || nodes_stack_.empty()) {
		throw std::logic_error("Problems obj isn't ready");
	}
	root_ = std::move(*nodes_stack_.back());
	return root_;
}

Builder::KeyItem Builder::Context::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder::DictItem Builder::Context::StartDict() {
    return builder_.StartDict();
}

Builder& Builder::Context::EndDict() {
    return builder_.EndDict();
}

Builder::ArrayItem Builder::Context::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::Context::EndArray() {
    return builder_.EndArray();
}
