#pragma once
#include "kcurl/easy.hpp"
#include "kcurl/http_status.hpp"

namespace kcurl::http {
enum class Verb : std::int8_t { Get, Post };

struct Query {
	std::string key{};
	std::string value{};
};

/// \brief Error of a fetch operation.
struct Error {
	/// \param status http Response Status.
	/// \param error_text Desired error text.
	/// \returns Formatted Error.
	[[nodiscard]] static auto from_response(Status status, std::string_view error_text) -> Error;

	CurlCode curl_code{};
	Status status{};
	std::string text{};
};

/// \brief Input parameter for fetch().
struct Request {
	/// \brief URL to fetch. Must be a valid URL.
	std::string base_url{};

	/// \brief User agent to use, if any.
	std::string user_agent{};
	/// \brief List of HTTP queries, if any.
	/// Appended to base_url if verb == Verb::Get, else added as post fields.
	std::vector<Query> queries{};
	/// \brief List of HTTP header queries, if any.
	/// Suffix a key with ':' to remove that default header.
	std::vector<Query> headers{};
	/// \brief Request method.
	Verb verb{Verb::Get};
};

/// \brief Successful response of a fetch operation.
/// Response is a class template to enable user-side
/// reuse for custom payloads (eg JSON).
template <typename Type>
struct Response {
	/// \returns Rewrapped payload with this response's status.
	template <typename T>
	[[nodiscard]] auto rewrap_as(T payload) const -> Response<T> {
		return Response<T>{.payload = std::move(payload), .status = status};
	}

	/// \returns Error with formatted error_text and this response's status.
	[[nodiscard]] auto rewrap_as_error(std::string_view const error_text) const -> Error {
		return Error::from_response(status, error_text);
	}

	Type payload{};
	Status status{};
};

/// \brief Result of a fetch operation.
/// Result is a class template to enable user-side
/// reuse for custom Reponse payloads (eg JSON).
template <typename Type>
using Result = std::expected<Response<Type>, Error>;

/// \param request http Request to convert.
/// \returns Corresponding easy::Request.
[[nodiscard]] auto to_easy_request(Request request) -> easy::Request;

/// \brief Perform easy::Request and interpret the easy::Result as an http::Result.
/// This lower-level function is exposed primarily for users to customize
/// some behavior after the conversion of an http::Request to an easy::Request
/// in the process of fetching the Response.
/// If that is not required, use fetch() directly.
/// \param request easy::Request to perform.
/// \returns http::Response as a ByteArray on success, else http::Error.
[[nodiscard]] auto perform(easy::Request const& request) -> Result<ByteArray>;

/// \brief Primary http API.
/// \param request http::Request to fetch.
/// \returns http::Response as a ByteArray on success, else http::Error.
[[nodiscard]] inline auto fetch(Request request) -> Result<ByteArray> {
	return http::perform(to_easy_request(std::move(request)));
}
} // namespace kcurl::http
