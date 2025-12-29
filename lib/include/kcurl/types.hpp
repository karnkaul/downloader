#pragma once
#include "kcurl/byte_array.hpp"
#include <cstdint>
#include <expected>
#include <string>

namespace kcurl {
// NOLINTNEXTLINE(performance-enum-size)
enum struct CurlCode : std::int64_t { Ok = 0 };

struct Error {
	/// \brief Code returned by libcurl.
	CurlCode code{};
	/// \brief Error text.
	std::string text{};
};

struct Request {
	/// \brief URL to fetch. Must be a valid URL.
	std::string url{};

	/// \brief User agent to use, if any.
	std::string user_agent{};
	/// \brief Concatenated string of post fields, if any.
	std::string post_fields{};
	/// \brief List of HTTP headers, if any.
	std::vector<std::string> headers{};
};

struct Response {
	/// \brief Response code.
	std::int64_t code{};
	/// \brief Response payload as bytes.
	ByteArray bytes{};
};

using Result = std::expected<Response, Error>;
} // namespace kcurl
