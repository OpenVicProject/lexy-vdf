#pragma once

#include <lexy-vdf/ParseError.hpp>

namespace lexy_vdf::errors {
	inline const ParseError make_no_file_error(const char* file_path) {
		std::string message;
		if (!file_path) {
			message = "File path not specified.";
		} else {
			message = "File '" + std::string(file_path) + "' was not found.";
		}

		return ParseError { ParseError::Type::Fatal, message, 1 };
	}
}