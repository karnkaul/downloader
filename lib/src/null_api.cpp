#include "kcurl/curl.hpp"
#include "kcurl/curl_code.hpp"
#include "kcurl/easy.hpp"

namespace kcurl {
auto Curl::default_flags() -> long { return {}; }

Curl::Curl(long const /*flags*/) {}

// NOLINTNEXTLINE(performance-trivially-destructible)
Curl::~Curl() = default;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto Curl::get_features() const -> Feature { return {}; }

auto easy::perform(Request const& /*request*/) -> Result {
	return std::unexpected{Error{.code = CurlCode{-1}, .text = "kcurl not built with curl enabled"}};
}
} // namespace kcurl
