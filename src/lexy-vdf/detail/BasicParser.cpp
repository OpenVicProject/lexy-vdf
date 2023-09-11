#include <iostream>
#include <ostream>

#include <lexy-vdf/detail/BasicParser.hpp>

#include "detail/NullBuff.hpp"

using namespace lexy_vdf;
using namespace lexy_vdf::detail;

BasicParser::BasicParser() : _error_stream(detail::cnull) {}

void BasicParser::set_error_log_to_null() {
	set_error_log_to(detail::cnull);
}

void BasicParser::set_error_log_to_stderr() {
	set_error_log_to(std::cerr);
}

void BasicParser::set_error_log_to_stdout() {
	set_error_log_to(std::cout);
}

void BasicParser::set_error_log_to(std::basic_ostream<char>& stream) {
	_error_stream = stream;
}

bool BasicParser::has_error() const {
	return !_errors.empty();
}

bool BasicParser::has_fatal_error() const {
	return _has_fatal_error;
}

bool BasicParser::has_warning() const {
	return !_warnings.empty();
}

const std::vector<lexy_vdf::ParseError>& BasicParser::get_errors() const {
	return _errors;
}

const std::vector<lexy_vdf::ParseWarning>& BasicParser::get_warnings() const {
	return _warnings;
}