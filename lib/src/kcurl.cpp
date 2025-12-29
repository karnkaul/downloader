#include "kcurl/easy.hpp"
#include "kcurl/http.hpp"
#include <curl/curl.h>
#include <algorithm>
#include <cstring>
#include <format>
#include <iterator>
#include <memory>
#include <utility>

namespace kcurl {
namespace {
struct CurlSlistDeleter {
	void operator()(curl_slist* ptr) const noexcept { curl_slist_free_all(ptr); }
};
using CurlSlist = std::unique_ptr<curl_slist, CurlSlistDeleter>;

class EasyHandle {
  public:
	explicit EasyHandle(Request const& request) : m_handle(curl_easy_init()) {
		set_callbacks();
		set_opt(CURLOPT_URL, request.url.c_str());
		if (!request.user_agent.empty()) { set_opt(CURLOPT_USERAGENT, request.user_agent.c_str()); }
		if (!request.post_fields.empty()) { set_opt(CURLOPT_POSTFIELDS, request.post_fields.c_str()); }
		add_headers(request.headers);
	}

	explicit EasyHandle(easy::Request const& request) : m_handle(curl_easy_init()) {
		set_callbacks();
		set_opt(CURLOPT_URL, request.url.c_str());
		if (!request.user_agent.empty()) { set_opt(CURLOPT_USERAGENT, request.user_agent.c_str()); }
		if (!request.post_fields.empty()) { set_opt(CURLOPT_POSTFIELDS, request.post_fields.c_str()); }
		add_headers(request.headers);
	}

	template <typename Type>
	void set_opt(CURLoption const opt, Type const value) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		curl_easy_setopt(m_handle.get(), opt, value);
	}

	[[nodiscard]] auto perform() -> std::expected<Response, Error> {
		auto const err = curl_easy_perform(m_handle.get());
		if (err != CURLE_OK) { return std::unexpected{Error{.code = CurlCode{err}, .text = std::move(m_error)}}; }

		auto response_code = long{};
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		curl_easy_getinfo(m_handle.get(), CURLINFO_RESPONSE_CODE, &response_code);
		return Response{.code = std::int64_t(response_code), .bytes = ByteArray{.bytes = std::move(m_bytes)}};
	}

	[[nodiscard]] auto perform2() -> std::expected<easy::Response, easy::Error> {
		auto const err = curl_easy_perform(m_handle.get());
		if (err != CURLE_OK) { return std::unexpected{easy::Error{.code = CurlCode{err}, .text = std::move(m_error)}}; }

		auto response_code = long{};
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		curl_easy_getinfo(m_handle.get(), CURLINFO_RESPONSE_CODE, &response_code);
		return easy::Response{.code = std::int64_t(response_code), .bytes = ByteArray{.bytes = std::move(m_bytes)}};
	}

  private:
	struct Deleter {
		void operator()(CURL* ptr) const noexcept { curl_easy_cleanup(ptr); }
	};

	void set_callbacks() {
		set_opt(CURLOPT_WRITEDATA, this);
		static auto write_fn = +[](void const* buffer, size_t /*size=1*/, size_t nmemb, void* userp) -> std::size_t {
			return static_cast<EasyHandle*>(userp)->on_write(std::span{static_cast<std::byte const*>(buffer), nmemb});
		};
		set_opt(CURLOPT_WRITEFUNCTION, write_fn);

		m_error.resize(CURL_ERROR_SIZE);
		set_opt(CURLOPT_ERRORBUFFER, m_error.data());
	}

	auto on_write(std::span<std::byte const> in) -> std::size_t {
		std::ranges::copy(in, std::back_inserter(m_bytes));
		return in.size();
	}

	void add_headers(std::span<std::string const> headers) {
		curl_slist* chunk{};
		for (auto const& text : headers) { chunk = curl_slist_append(chunk, text.c_str()); }
		m_headers.reset(chunk);
		set_opt(CURLOPT_HTTPHEADER, m_headers.get());
	}

	std::unique_ptr<CURL, Deleter> m_handle{};
	CurlSlist m_headers{};

	std::vector<std::byte> m_bytes{};
	std::string m_error{};
};
} // namespace

namespace http {
namespace {
void append_to(std::string& out, Query const& query) {
	if (query.key.empty()) { return; }
	if (!query.value.empty()) {
		std::format_to(std::back_inserter(out), "{}={}&", query.key, query.value);
	} else {
		out += query.key;
	}
	out += '&';
}

[[nodiscard]] auto serialize_queries(std::span<Query const> queries) -> std::string {
	if (queries.empty()) { return {}; }
	auto ret = std::string{};
	for (auto const& query : queries) { append_to(ret, query); }
	if (ret.ends_with('&')) { ret.pop_back(); }
	return ret;
}

void append_queries_to(std::string& out_url, std::span<Query const> queries) {
	auto const serialized_queries = serialize_queries(queries);
	if (!serialized_queries.empty()) { std::format_to(std::back_inserter(out_url), "?{}", serialized_queries); }
}

[[nodiscard]] auto serialize_headers(std::span<Query> queries) -> std::vector<std::string> {
	auto ret = std::vector<std::string>{};
	for (auto& [key, value] : queries) {
		if (key.empty()) { continue; }
		if (value.empty() && !key.ends_with(':')) { key.push_back(';'); }
		key += value;
		ret.push_back(std::move(key));
	}
	return ret;
}

[[nodiscard]] auto to_error_text(CurlCode const code, std::string_view const error_text) -> std::string {
	return std::format("curl error ({}):\n{}", std::to_underlying(code), error_text);
}

[[nodiscard]] auto to_error_text(Status const& status, std::string_view const error_text) -> std::string {
	auto const prefix = [status] -> std::string_view {
		switch (status.get_category()) {
		case Status::Category::ClientError: return "http client";
		case Status::Category::ServerError: return "http server";
		default: return "http";
		}
	}();
	return std::format("{} error ({}):\n{}", prefix, std::to_underlying(status.get_code()), error_text);
}
} // namespace
} // namespace http

auto easy::perform(Request const& request) -> Result {
	if (request.url.empty()) { return {}; }
	auto handle = EasyHandle{request};
	return handle.perform2();
}

auto http::to_easy_request(Request request) -> easy::Request {
	auto ret = easy::Request{
		.url = std::move(request.base_url),
		.user_agent = std::move(request.user_agent),
	};

	switch (request.verb) {
	case Verb::Get: append_queries_to(ret.url, request.queries); break;
	case Verb::Post: ret.post_fields = serialize_queries(request.queries); break;
	default: break;
	}

	ret.headers = serialize_headers(request.headers);

	return ret;
}

auto http::fetch(easy::Request const& request) -> Result {
	auto response = easy::perform(request);

	if (!response) {
		return std::unexpected{Error{
			.curl_code = response.error().code,
			.text = to_error_text(response.error().code, response.error().text),
		}};
	}

	auto ret = Response{.bytes = std::move(response->bytes), .status = Status{response->code}};
	if (ret.status.is_error()) {
		auto error_text = to_error_text(ret.status, ret.bytes.as_string_view());
		return std::unexpected{ret.rewrap_as_error(std::move(error_text))};
	}

	return ret;
}
} // namespace kcurl
