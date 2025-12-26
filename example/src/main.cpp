#include "downloader/build_version.hpp"
#include "downloader/curl.hpp"
#include "downloader/download.hpp"
#include <filesystem>
#include <print>
#include <span>

namespace example {
namespace {
namespace fs = std::filesystem;

[[nodiscard]] constexpr auto is_option(std::string_view const arg) -> bool {
	return arg.starts_with('-') || arg.starts_with("--");
}

auto run(int const argc, char const* const* const argv) -> int {
	auto args = std::span{argv, std::size_t(argc)};
	auto const exe_name = [&] {
		if (!args.empty()) {
			auto ret = fs::path{args.front()}.stem().string();
			args = args.subspan(1);
			return ret;
		}
		return std::string{"<exe>"};
	}();

	auto const print_usage = [&exe_name](std::FILE* const out) {
		std::println(out, "Usage: {} [URL]", exe_name);
		return out == stderr ? EXIT_FAILURE : EXIT_SUCCESS;
	};

	auto request = downloader::Request{.url = "http://example.org"};
	if (!args.empty()) {
		if (args.size() > 1 || is_option(args.front())) { return print_usage(stderr); }
		request.url = args.front();
	}

	std::println("downloader {}", downloader::build_version_v);
	std::println("downloading: {}...", request.url);

	auto const curl = downloader::Curl{};
	auto const result = downloader::download(request);

	if (!result) {
		auto const& error = result.error();
		std::println(stderr, "error ({}): {}", std::int64_t(error.code), error.text);
		return EXIT_FAILURE;
	}

	auto const text = downloader::as_string_view(result->bytes);
	std::println("success ({})\n{}", std::int64_t(result->code), text);

	return EXIT_SUCCESS;
}
} // namespace
} // namespace example

auto main(int argc, char** argv) -> int {
	try {
		return example::run(argc, argv);
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}
}
