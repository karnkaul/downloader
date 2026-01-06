#pragma once
#include <curl/curl.h>
#include <cstdint>
#include <format>

namespace kcurl {
/// \brief Top-level RAII wrapper for curl.
/// Use if libcurl initialization/shutdown is not already handled.
class Curl {
  public:
	enum Feature : std::uint8_t {
		None = 0,
		TLS = 1 << 0,
		IPv6 = 1 << 1,
		Win32Unicode = 1 << 2,
		UnixSockets = 1 << 3,
		Http2 = 1 << 4,
		Http3 = 1 << 5,
		LargeFile = 1 << 6,
	};

	static constexpr auto flags_v = long{CURL_GLOBAL_DEFAULT};

	Curl(Curl const&) = delete;
	Curl(Curl&&) = delete;
	Curl& operator=(Curl const&) = delete;
	Curl& operator=(Curl&&) = delete;

	explicit Curl(long const flags = flags_v) { curl_global_init(flags); }
	~Curl() { curl_global_cleanup(); }

	[[nodiscard]] auto get_features() const -> Feature;
};
} // namespace kcurl

template <>
struct std::formatter<kcurl::Curl::Feature> {
	template <class ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return ctx.begin();
	}

	static auto format(kcurl::Curl::Feature const& flags, format_context& fc) -> format_context::iterator;
};
