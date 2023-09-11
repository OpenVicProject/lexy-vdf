#pragma once

#include <optional>

#include <lexy-vdf/KeyValues.hpp>
#include <lexy-vdf/ParseWarning.hpp>
#include <lexy-vdf/detail/OptionalConstexpr.hpp>

namespace lexy_vdf::warnings {
	LVDF_OPTIONAL_CONSTEXPR std::optional<ParseWarning> merge_check(std::string_view file, KeyValues::MergeError merge_error) {
		switch (merge_error) {
			case KeyValues::MergeError::FileMissing:
				return ParseWarning { "Could not find '" + std::string(file) + "'.", 1 };
			case KeyValues::MergeError::ParseFail:
				return ParseWarning { '"' + std::string(file) + "' could not be parsed.", 2 };
			default:
				return std::nullopt;
		}
	}
}