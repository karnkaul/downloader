#pragma once
#include "downloader/http/status.hpp"
#include <expected>
#include <string>
#include <vector>

namespace downloader::http {
enum class ErrorType : std::int8_t { Http, Curl };

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
	[[nodiscard]] auto build_url() const -> std::string;

	std::string base_url{};
	std::vector<Query> queries{};
	std::string user_agent{};
};

template <typename PayloadT>
struct Response {
	template <typename Type>
	[[nodiscard]] auto rewrap(Type payload) const -> Response<Type> {
		return Response<Type>{.payload = std::move(payload), .status = status};
	}

	[[nodiscard]] auto rewrap_as_error(std::string error_text) const -> Error {
		return Error{
			.code = std::int64_t(status.get_code()),
			.text = std::move(error_text),
			.type = http::ErrorType::Http,
		};
	}

	PayloadT payload{};
	Status status{};
};

template <typename Type>
using Result = std::expected<Response<Type>, Error>;
} // namespace downloader::http
