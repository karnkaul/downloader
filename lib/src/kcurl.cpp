#include "kcurl/curl.hpp"
#include "kcurl/easy.hpp"
#include "kcurl/http.hpp"
#include <algorithm>
#include <format>
#include <iterator>
#include <span>
#include <string_view>
#include <utility>

namespace kcurl {
namespace http {
namespace {
struct EscapeEntry {
	char ch{};
	std::string_view str{};
};

constexpr auto escape_entries_v = std::array{
	EscapeEntry{.ch = ' ', .str = "%20"},  EscapeEntry{.ch = '#', .str = "%23"}, EscapeEntry{.ch = '$', .str = "%24"},
	EscapeEntry{.ch = '%', .str = "%25"},  EscapeEntry{.ch = '&', .str = "%26"}, EscapeEntry{.ch = '@', .str = "%40"},
	EscapeEntry{.ch = '`', .str = "%60"},  EscapeEntry{.ch = '/', .str = "%2F"}, EscapeEntry{.ch = ':', .str = "%3A"},
	EscapeEntry{.ch = ';', .str = "%3B"},  EscapeEntry{.ch = '<', .str = "%3C"}, EscapeEntry{.ch = '=', .str = "%3D"},
	EscapeEntry{.ch = '>', .str = "%3F"},  EscapeEntry{.ch = '?', .str = "%3F"}, EscapeEntry{.ch = '[', .str = "%5B"},
	EscapeEntry{.ch = '\\', .str = "%5C"}, EscapeEntry{.ch = ']', .str = "%5D"}, EscapeEntry{.ch = '^', .str = "%5E"},
	EscapeEntry{.ch = '{', .str = "%7B"},  EscapeEntry{.ch = '|', .str = "%7C"}, EscapeEntry{.ch = '}', .str = "%7D"},
	EscapeEntry{.ch = '~', .str = "%7E"},  EscapeEntry{.ch = '"', .str = "%22"}, EscapeEntry{.ch = '\'', .str = "%27"},
	EscapeEntry{.ch = '+', .str = "%2B"},
};

void append_to(std::string& out, Query const& query) {
	if (query.key.empty()) { return; }
	if (!query.value.empty()) {
		std::format_to(std::back_inserter(out), "{}={}", query.key, query.value);
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

[[nodiscard]] auto to_error(easy::Error const& in) -> Error {
	return Error{.curl_code = in.code, .text = to_error_text(in.code, in.text)};
}

[[nodiscard]] auto to_error(Status const status, std::string_view const error_text) -> Error {
	return Error{.status = status, .text = to_error_text(status, error_text)};
}

[[nodiscard]] auto to_response(easy::Response in) -> Response<ByteArray> {
	return Response{.payload = std::move(in.bytes), .status = Status{in.code}};
}
} // namespace

auto Error::from_response(Status const status, std::string_view const error_text) -> Error {
	return to_error(status, error_text);
}
} // namespace http

auto http::escape(std::string_view const text) -> std::string {
	auto ret = std::string{};
	ret.reserve(text.size());
	for (char const ch : text) {
		// NOLINTNEXTLINE(readability-qualified-auto)
		auto const it = std::ranges::find_if(escape_entries_v, [ch](EscapeEntry const& e) { return e.ch == ch; });
		if (it != escape_entries_v.end()) {
			ret.append(it->str);
			continue;
		}

		ret.push_back(ch);
	}
	return ret;
}

auto http::unescape(std::string_view escaped) -> std::string {
	auto ret = std::string{};
	ret.reserve(escaped.size());
	auto const replace = [&] {
		auto const pred = [&escaped](EscapeEntry const& e) { return escaped.starts_with(e.str); };
		// NOLINTNEXTLINE(readability-qualified-auto)
		auto const it = std::ranges::find_if(escape_entries_v, pred);
		if (it == escape_entries_v.end()) { return false; }
		ret.push_back(it->ch);
		escaped.remove_prefix(it->str.size());
		return true;
	};
	while (!escaped.empty()) {
		if (escaped.starts_with('%') && replace()) { continue; }
		ret.push_back(escaped.front());
		escaped.remove_prefix(1);
	}
	return ret;
}

auto http::to_easy_request(Request request) -> easy::Request {
	auto ret = easy::Request{
		.url = std::move(request.base_url),
		.user_agent = std::move(request.user_agent),
		.flags = request.flags,
	};

	switch (request.verb) {
	case Verb::Get: append_queries_to(ret.url, request.queries); break;
	case Verb::Post: ret.post_fields = serialize_queries(request.queries); break;
	default: break;
	}

	ret.headers = serialize_headers(request.headers);

	return ret;
}

auto http::perform(easy::Request const& request) -> Result<ByteArray> {
	auto easy_result = easy::perform(request);
	if (!easy_result) { return std::unexpected{to_error(easy_result.error())}; }

	auto response = to_response(std::move(easy_result).value());
	if (response.status.is_error()) {
		return std::unexpected{response.rewrap_as_error(response.payload.as_string_view())};
	}

	return response;
}
} // namespace kcurl

auto std::formatter<kcurl::Curl::Feature>::format(kcurl::Curl::Feature const flags, format_context& fc)
	-> format_context::iterator {
	auto first = true;
	auto const append = [&](std::string_view const text) {
		if (!first) {
			format_to(fc.out(), "|{}", text);
		} else {
			format_to(fc.out(), "{}", text);
		}
		first = false;
	};

	using Feature = kcurl::Curl::Feature;
	if ((flags & Feature::TLS) == Feature::TLS) { append("TLS"); }
	if ((flags & Feature::IPv6) == Feature::IPv6) { append("IPv6"); }
	if ((flags & Feature::Win32Unicode) == Feature::Win32Unicode) { append("Win32Unicode"); }
	if ((flags & Feature::UnixSockets) == Feature::UnixSockets) { append("UnixSockets"); }
	if ((flags & Feature::Http2) == Feature::Http2) { append("HTTP2"); }
	if ((flags & Feature::Http3) == Feature::Http3) { append("HTTP3"); }
	if ((flags & Feature::LargeFile) == Feature::LargeFile) { append("LargeFile"); }
	return fc.out();
}
