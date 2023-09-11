#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include <lexy-vdf/KeyValues.hpp>
#include <lexy-vdf/Parser.hpp>

using namespace lexy_vdf;

std::string_view trim(std::string_view str) {
	std::string_view::iterator begin = str.begin();
	std::string_view::iterator end = str.end();
	for (;; begin++) {
		if (begin == end) return std::string_view();
		if (!std::isspace(*begin)) break;
	}
	end--;
	for (;; end--) {
		if (end == begin) return std::string_view();
		if (!std::isspace(*end)) break;
	}
	return std::string_view(&*begin, std::distance(begin, end));
}

bool insenitive_trim_eq(std::string_view lhs, std::string_view rhs) {
	lhs = trim(lhs);
	rhs = trim(rhs);
	return std::equal(
		lhs.begin(), lhs.end(),
		rhs.begin(), rhs.end(),
		[](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

KeyValues::KeyValues(std::initializer_list<value_type> list) : base_type(list) {
}

std::unique_ptr<KeyValues> KeyValues::from_buffer(const char* data, std::size_t size) {
	Parser parser = Parser::from_buffer(data, size);
	if (parser.has_error()) return nullptr;
	return std::unique_ptr<KeyValues>(parser.release_key_values());
}

std::unique_ptr<KeyValues> KeyValues::from_buffer(const char* start, const char* end) {
	Parser parser = Parser::from_buffer(start, end);
	if (parser.has_error()) return nullptr;
	return std::unique_ptr<KeyValues>(parser.release_key_values());
}

std::unique_ptr<KeyValues> KeyValues::from_string(const std::string_view string) {
	Parser parser = Parser::from_string(string);
	if (parser.has_error()) return nullptr;
	return std::unique_ptr<KeyValues>(parser.release_key_values());
}

std::unique_ptr<KeyValues> KeyValues::from_file(std::string_view path) {
	Parser parser = Parser::from_file(path);
	if (parser.has_error()) return nullptr;
	return std::unique_ptr<KeyValues>(parser.release_key_values());
}

std::unique_ptr<KeyValues> KeyValues::from_file(const std::filesystem::path& path) {
	Parser parser = Parser::from_file(path);
	if (parser.has_error()) return nullptr;
	return std::unique_ptr<KeyValues>(parser.release_key_values());
}

KeyValues::MergeError KeyValues::MergeWith(const std::filesystem::path& p_path) {
	Parser parser;
	parser.load_from_file(p_path);
	if (parser.has_error()) return MergeError::FileMissing;
	if (!parser.parse()) return MergeError::ParseFail;
	AppendKeyValues(*parser.get_key_values());
	return MergeError::Success;
}

KeyValues& KeyValues::AppendKeyValues(const KeyValues& p_key_values) {
	reserve(p_key_values.size());
	for (const auto& value : p_key_values) {
		emplace(value);
	}
	return *this;
}

std::int32_t KeyValues::GetInt(KeyObserverType p_key, std::int32_t p_default_value) const {
	const_iterator value = find(p_key);
	const std::int32_t* result = std::get_if<std::int32_t>(&(value->second));
	if (!result) return p_default_value;
	return *result;
}

std::float_t KeyValues::GetFloat(KeyObserverType p_key, std::float_t p_default_value) const {
	const_iterator value = find(p_key);
	const std::float_t* result = std::get_if<std::float_t>(&(value->second));
	if (!result) return p_default_value;
	return *result;
}

std::string_view KeyValues::GetString(KeyObserverType p_key, std::string_view p_default_value) const {
	const_iterator value = find(p_key);
	const std::string* result = std::get_if<std::string>(&(value->second));
	if (!result) return p_default_value;
	return *result;
}

bool KeyValues::GetBool(KeyObserverType p_key, bool p_default_value) const {
	const_iterator value = find(p_key);
	if (value == end()) return p_default_value;
	return std::visit([p_default_value](auto&& arg) -> bool {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::float_t>) {
			return arg;
		} else if constexpr (std::is_same_v<T, std::string>) {
			return insenitive_trim_eq("true", arg);
		} else if constexpr (std::is_same_v<T, KeyValues>) {
			if (arg.empty()) return false;
			return true;
		}
		return false;
	},
		value->second);
}