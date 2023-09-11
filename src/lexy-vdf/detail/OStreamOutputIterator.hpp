#pragma once

#include <ostream>

namespace lexy_vdf::detail {
	struct OStreamOutputIterator {
		std::reference_wrapper<std::ostream> _stream;

		auto operator*() const noexcept {
			return *this;
		}
		auto operator++(int) const noexcept {
			return *this;
		}

		OStreamOutputIterator& operator=(char c) {
			_stream.get().put(c);
			return *this;
		}
	};
}