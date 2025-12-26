#pragma once
#include "downloader/types.hpp"
#include <span>

namespace downloader {
/// \returns Result of operation.
[[nodiscard]] auto download(Request const& request) -> Result;

/// \returns View of byte array as a string.
[[nodiscard]] constexpr auto as_string_view(std::span<std::byte const> bytes) -> std::string_view {
	void const* data = bytes.data();
	if (!data) { return {}; }
	return std::string_view{static_cast<char const*>(data), bytes.size()};
}
} // namespace downloader
