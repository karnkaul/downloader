#pragma once
#include "downloader/byte_array.hpp"
#include "downloader/http/status.hpp"
#include <expected>
#include <string>
#include <vector>

namespace downloader::http {
enum class ErrorType : std::int8_t { Http, Curl };

enum class Verb : std::int8_t { Get, Post };

struct Error {
	std::int64_t code{};
	std::string text{};
	ErrorType type{ErrorType::Http};
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
		return Error{.code = std::int64_t(status.get_code()), .text = std::move(error_text), .type = ErrorType::Http};
	}

	ByteArray bytes{};
	Status status{};
};

using Result = std::expected<Response, Error>;
} // namespace downloader::http
