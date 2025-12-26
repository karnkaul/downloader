#include "downloader/download.hpp"
#include <curl/curl.h>
#include <algorithm>
#include <iterator>
#include <memory>

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
		if (err != CURLE_OK) { return std::unexpected(Error{.code = CurlCode{err}, .text = std::move(m_error)}); }

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
} // namespace downloader

auto downloader::download(Request const& request) -> Result {
	auto handle = EasyHandle{request};
	return handle.perform();
}
