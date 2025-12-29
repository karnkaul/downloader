#pragma once
#include "kcurl/byte_array.hpp"
#include "kcurl/types.hpp"
#include <expected>
#include <string>

namespace kcurl::easy {
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

/// \returns Result of operation.
[[nodiscard]] auto perform(Request const& request) -> Result;
} // namespace kcurl::easy
