#pragma once
#include "downloader/types.hpp"

namespace downloader {
/// \returns Result of operation.
[[nodiscard]] auto perform(Request const& request) -> Result;
} // namespace downloader
