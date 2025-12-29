#pragma once
#include "downloader/http/types.hpp"

namespace downloader::http {
[[nodiscard]] auto fetch(Request request) -> Result;
} // namespace downloader::http
