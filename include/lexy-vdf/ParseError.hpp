#pragma once

#include <string>

#include <lexy-vdf/ParseData.hpp>

namespace lexy_vdf {
	struct ParseError {
		const enum class Type : unsigned char {
			Recoverable,
			Fatal
		} type;
		const std::string message;
		const int error_value;
		const ParseData parse_data;
		const unsigned int start_line;
		const unsigned int start_column;
	};

}