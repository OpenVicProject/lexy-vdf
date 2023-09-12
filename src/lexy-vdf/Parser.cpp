#include <functional>
#include <string_view>

#include <lexy-vdf/Parser.hpp>

#include <lexy/action/parse.hpp>
#include <lexy/encoding.hpp>

#include "lexy-vdf/KeyValues.hpp"

#include "Grammar.hpp"
#include "detail/BasicBufferHandler.hpp"
#include "detail/LexyReportError.hpp"
#include "detail/OStreamOutputIterator.hpp"

using namespace lexy_vdf;

class Parser::BufferHandler final : public detail::BasicBufferHandler<lexy::utf8_char_encoding> {
public:
	template<typename Node, typename ParseState, typename ErrorCallback>
	std::optional<std::vector<ParseError>> parse(ParseState& state, const ErrorCallback& callback) {
		auto result = lexy::parse<Node>(this->_buffer, state, callback);
		if (!result) {
			return result.errors();
		}
		_key_values = std::move(result.value());
		return std::nullopt;
	}

	KeyValues* get_key_values() { return _key_values; }

private:
	KeyValues* _key_values;
};

/// BufferHandler ///

Parser::Parser()
	: _buffer_handler(std::make_unique<BufferHandler>()) {
	set_error_log_to_stderr();
	set_default_conditions();
	_parser_state.parse_warnings = &_warnings;
}

Parser::Parser(Parser&&) = default;
Parser& Parser::operator=(Parser&&) = default;
Parser::~Parser() = default;

Parser Parser::from_buffer(const char* data, std::size_t size) {
	Parser result;
	return std::move(result.load_from_buffer(data, size));
}
Parser Parser::from_buffer(const char* start, const char* end) {
	Parser result;
	return std::move(result.load_from_buffer(start, end));
}

Parser Parser::from_string(const std::string_view string) {
	Parser result;
	return std::move(result.load_from_string(string));
}

Parser Parser::from_file(std::string_view path) {
	Parser result;
	return std::move(result.load_from_file(path));
}

Parser Parser::from_file(const std::filesystem::path& path) {
	Parser result;
	return std::move(result.load_from_file(path));
}

///
/// @brief Executes a function on _buffer_handler that is expected to load a buffer
///
/// Expected Use:
/// @code {.cpp}
///	_run_load_func(&BufferHandler::<load_function>, <arguments>);
/// @endcode
///
/// @tparam Type
/// @tparam Args
/// @param func
/// @param args
///
template<typename... Args>
constexpr void Parser::_run_load_func(detail::LoadCallback<BufferHandler, Args...> auto func, Args... args) {
	_warnings.clear();
	_errors.clear();
	_has_fatal_error = false;
	if (auto error = func(_buffer_handler.get(), std::forward<Args>(args)...); error) {
		_has_fatal_error = error.value().type == ParseError::Type::Fatal;
		_errors.push_back(error.value());
		_error_stream.get() << "Error: " << _errors.back().message << '\n';
	}
}

constexpr Parser& Parser::load_from_buffer(const char* data, std::size_t size) {
	_run_load_func(std::mem_fn(&BufferHandler::load_buffer_size), data, size);
	return *this;
}

constexpr Parser& Parser::load_from_buffer(const char* start, const char* end) {
	_run_load_func(std::mem_fn(&BufferHandler::load_buffer), start, end);
	return *this;
}

constexpr Parser& Parser::load_from_string(std::string_view string) {
	return load_from_buffer(string.data(), string.size());
}

constexpr Parser& Parser::load_from_file(const char* path) {
	_file_path = path;
	_run_load_func(std::mem_fn(&BufferHandler::load_file), path);
	return *this;
}

Parser& Parser::load_from_file(const char* path, const Parser& root) {
	_file_path = path;
	_run_load_func(std::mem_fn(&BufferHandler::load_file), path);
	_parser_state.conditionals = root._parser_state.conditionals;
	return *this;
}

Parser& Parser::load_from_file(const std::filesystem::path& path) {
	return load_from_file(path.string());
}

bool Parser::parse() {
	if (!_buffer_handler->is_valid()) {
		return false;
	}

	std::optional<std::vector<ParseError>> errors;
	errors = _buffer_handler->template parse<lexy_vdf::grammar::File>(_parser_state, lexy_vdf::detail::ReportError.path(_file_path).to(detail::OStreamOutputIterator { _error_stream }));
	if (errors) {
		_errors.reserve(errors->size());
		for (auto& err : errors.value()) {
			_has_fatal_error |= err.type == ParseError::Type::Fatal;
			_errors.push_back(err);
		}
		return false;
	}
	_key_values.reset(_buffer_handler->get_key_values());
	return true;
}

const KeyValues* Parser::get_key_values() {
	return _key_values.get();
}

KeyValues* Parser::release_key_values() {
	return _key_values.release();
}

const Parser::State Parser::get_parse_state() const {
	return _parser_state;
}

void Parser::set_default_conditions() {
	// Sourced from https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/tier1/KeyValues.cpp
	// Platform conditions sourced from https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/public/tier0/platform.h#L85
#if defined(_X360)
	add_condition("X360");
#elif defined(WIN32)
	add_condition("WIN32");
	add_condition("WINDOWS");
#elif defined(__APPLE__)
	add_condition("WIN32");
	add_condition("POSIX");
	add_condition("OSX");
#elif defined(__linux__)
	add_condition("WIN32");
	add_condition("POSIX");
	add_condition("LINUX");
#else
	// Any other default conditions
#endif
}

void Parser::clear_conditions() {
	_parser_state.conditionals.clear();
}

void Parser::add_condition(std::string_view conditional) {
	_parser_state.conditionals.insert(conditional.data());
}

bool Parser::remove_condition(std::string_view conditional) {
	auto found = _parser_state.conditionals.find(conditional);
	if (found == _parser_state.conditionals.end()) return false;
	return _parser_state.conditionals.erase(*found) == 1;
}

bool Parser::has_condition(std::string_view conditional) const {
	return _parser_state.has_condition(conditional);
}