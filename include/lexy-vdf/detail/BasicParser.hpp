#pragma once

#include <vector>

#include <lexy-vdf/ParseError.hpp>
#include <lexy-vdf/ParseWarning.hpp>
#include <lexy-vdf/detail/Concepts.hpp>

namespace lexy_vdf::detail {
	class BasicParser {
	public:
		BasicParser();

		void set_error_log_to_null();
		void set_error_log_to_stderr();
		void set_error_log_to_stdout();
		void set_error_log_to(std::basic_ostream<char>& stream);

		bool has_error() const;
		bool has_fatal_error() const;
		bool has_warning() const;

		const std::vector<ParseError>& get_errors() const;
		const std::vector<ParseWarning>& get_warnings() const;

	protected:
		std::vector<ParseError> _errors;
		std::vector<ParseWarning> _warnings;

		std::reference_wrapper<std::ostream> _error_stream;
		const char* _file_path;
		bool _has_fatal_error = false;
	};
}