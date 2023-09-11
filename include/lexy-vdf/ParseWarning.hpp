#pragma once

#include <string>

namespace lexy_vdf {
	struct ParseWarning {
		const std::string message;
		const int warning_value;
	};
}