#pragma once
#include "kcurl/byte_array.hpp"
#include "kcurl/curl_code.hpp"
#include <cstdint>
#include <expected>
#include <string>

namespace kcurl::easy {
/// \brief Error of a perform operation.
struct Error {
	/// \brief Code returned by libcurl.
	CurlCode code{};
	/// \brief Error text.
	std::string text{};
};

/// \brief Input parameter for perform().
struct Request {
	enum Flag : std::uint8_t {
		None = 0,
		SkipPeerVerification = 1 << 0,
		SkipHostnameVerification = 1 << 1,
	};

	/// \brief URL to fetch. Must be a valid URL.
	std::string url{};

	/// \brief User agent to use, if any.
	std::string user_agent{};
	/// \brief Concatenated string of post fields, if any.
	std::string post_fields{};
	/// \brief List of HTTP headers, if any.
	std::vector<std::string> headers{};
	/// \brief Request flags.
	Flag flags{None};
};

/// \brief Successful response of a perform operation.
struct Response {
	/// \brief Response code.
	std::int64_t code{};
	/// \brief Response payload as bytes.
	ByteArray bytes{};
};

/// \brief Result of a fetch operation.
using Result = std::expected<Response, Error>;

/// \brief Primary easy API.
/// \param request easy::Request to perform.
/// \returns easy::Response as a ByteArray on success, else easy::Error.
[[nodiscard]] auto perform(Request const& request) -> Result;
} // namespace kcurl::easy
