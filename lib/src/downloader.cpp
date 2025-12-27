#include "downloader/download.hpp"
#include "downloader/http/gateway.hpp"
#include <curl/curl.h>
#include <algorithm>
#include <cstring>
#include <format>
#include <iterator>
#include <memory>
#include <utility>

namespace downloader {
namespace {
class EasyHandle {
  public:
	explicit EasyHandle(Request const& request) : m_handle(curl_easy_init()) {
		set_callbacks();
		set_opt(CURLOPT_URL, request.url.c_str());
		if (!request.user_agent.empty()) { set_opt(CURLOPT_USERAGENT, request.user_agent.c_str()); }
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
		return Response{.code = std::int64_t(response_code), .bytes = std::move(m_bytes)};
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

	std::unique_ptr<CURL, Deleter> m_handle{};

	std::vector<std::byte> m_bytes{};
	std::string m_error{};
};
} // namespace

namespace http {
namespace {
template <typename PayloadT>
[[nodiscard]] auto wrap_response(PayloadT payload, std::int64_t const status_code) {
	return Response<PayloadT>{.payload = std::move(payload), .status = Status{status_code}};
}

[[nodiscard]] auto to_http_error_text(CurlCode const code, std::string_view const error_text) -> std::string {
	return std::format("curl error ({}):\n{}", std::to_underlying(code), error_text);
}

[[nodiscard]] auto to_http_error_text(Status const& status, std::string_view const error_text) -> std::string {
	auto const prefix = [status] -> std::string_view {
		switch (status.get_category()) {
		case Status::Category::ClientError: return "http client";
		case Status::Category::ServerError: return "http server";
		default: return "http";
		}
	}();
	return std::format("{} error ({}):\n{}", prefix, std::to_underlying(status.get_code()), error_text);
}

[[nodiscard]] auto wrap_error(downloader::Error const& error) {
	return std::unexpected{Error{
		.code = std::int64_t(error.code),
		.text = to_http_error_text(error.code, error.text),
		.type = ErrorType::Curl,
	}};
}
} // namespace

auto Request::build_url() const -> std::string {
	auto ret = base_url;
	if (!queries.empty()) {
		ret += '?';
		for (auto const& [key, value] : queries) { std::format_to(std::back_inserter(ret), "{}={}&", key, value); }
		ret.pop_back();
	}
	return ret;
}

auto Gateway::get_bytes(Request request) const -> Result<std::vector<std::byte>> {
	if (request.base_url.empty()) { return {}; }
	auto const download_request = downloader::Request{
		.url = request.build_url(),
		.user_agent = std::move(request.user_agent),
	};

	auto response = perform_download(download_request);
	if (!response) { return wrap_error(response.error()); }

	auto ret = wrap_response(std::move(response->bytes), response->code);
	if (ret.status.is_error()) {
		auto error_text = to_http_error_text(ret.status, as_string_view(ret.payload));
		return std::unexpected{ret.rewrap_as_error(std::move(error_text))};
	}

	return ret;
}

auto Gateway::get_string(Request request) const -> Result<std::string> {
	auto response = get_bytes(std::move(request));
	if (!response) { return std::unexpected{std::move(response.error())}; }

	auto ret = std::string{};
	if (!response->payload.empty()) {
		ret.resize(response->payload.size());
		std::memcpy(ret.data(), response->payload.data(), ret.size());
	}

	return response->rewrap(std::move(ret));
}

auto Gateway::perform_download(downloader::Request const& request) const -> downloader::Result {
	return download(request);
}
} // namespace http
} // namespace downloader

auto downloader::download(Request const& request) -> Result {
	auto handle = EasyHandle{request};
	return handle.perform();
}
