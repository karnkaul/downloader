#pragma once
#include "kcurl/easy.hpp"
#include "kcurl/http_status.hpp"

namespace kcurl::http {
enum class Verb : std::int8_t { Get, Post };

struct Error {
	CurlCode curl_code{};
	Status status{};
	std::string text{};
};

struct Query {
	std::string key{};
	std::string value{};
};

struct Request {
	/// \brief URL to fetch. Must be a valid URL.
	std::string base_url{};

	/// \brief User agent to use, if any.
	std::string user_agent{};
	/// \brief List of HTTP queries.
	/// Appended to base_url if verb == Verb::Get, else added as post fields.
	std::vector<Query> queries{};
	/// \brief Suffix a key with ':' to remove that default header.
	std::vector<Query> headers{};
	/// \brief Request method.
	Verb verb{Verb::Get};
};

struct Response {
	/// \returns Error with given error_text and this response's status.
	[[nodiscard]] auto rewrap_as_error(std::string error_text) const -> Error {
		return Error{.status = status, .text = std::move(error_text)};
	}

	ByteArray bytes{};
	Status status{};
};

using Result = std::expected<Response, Error>;

[[nodiscard]] auto to_easy_request(Request request) -> easy::Request;

[[nodiscard]] auto fetch(easy::Request const& request) -> Result;

[[nodiscard]] inline auto fetch(Request request) -> Result { return fetch(to_easy_request(std::move(request))); }
} // namespace kcurl::http
