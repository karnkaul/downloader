#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace downloader {
// NOLINTNEXTLINE(performance-enum-size)
enum struct CurlCode : std::int64_t { Ok = 0 };

struct Error {
	/// \brief Code returned by libcurl.
	CurlCode code{};
	/// \brief Error text.
	std::string text{};
};

struct Request {
	/// \brief URL to fetch.
	std::string url{};
	/// \brief User agent to use.
	std::string user_agent{};
};

struct Response {
	/// \brief Response code.
	std::int64_t code{};
	/// \brief Response payload as bytes.
	std::vector<std::byte> bytes{};
};

using Result = std::expected<Response, Error>;
} // namespace downloader
