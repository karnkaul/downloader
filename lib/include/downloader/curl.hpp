#pragma once
#include <curl/curl.h>

namespace downloader {
/// \brief Top-level RAII wrapper for curl.
/// Use if libcurl initialization/shutdown is not already handled.
class Curl {
  public:
	Curl(Curl const&) = delete;
	Curl(Curl&&) = delete;
	Curl& operator=(Curl const&) = delete;
	Curl& operator=(Curl&&) = delete;

	explicit Curl() { curl_global_init(CURL_GLOBAL_DEFAULT); }
	~Curl() { curl_global_cleanup(); }
};
} // namespace downloader
