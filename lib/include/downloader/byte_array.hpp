#pragma once
#include <cstddef>
#include <string_view>
#include <vector>

namespace downloader {
struct ByteArray {
	/// \returns Bytes reinterpreted as a string_view.
	[[nodiscard]] constexpr auto as_string_view() const -> std::string_view {
		void const* data = bytes.data();
		if (!data) { return {}; }
		return std::string_view{static_cast<char const*>(data), bytes.size()};
	}

	std::vector<std::byte> bytes{};
};
} // namespace downloader
