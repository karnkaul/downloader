#pragma once
#include "downloader/http/types.hpp"
#include "downloader/types.hpp"

namespace downloader::http {
/// \brief Customizable http wrapper over download().
class Gateway {
  public:
	virtual ~Gateway() = default;

	Gateway() = default;
	Gateway(Gateway const&) = default;
	Gateway(Gateway&&) = default;
	Gateway& operator=(Gateway const&) = default;
	Gateway& operator=(Gateway&&) = default;

	[[nodiscard]] auto get_bytes(Request request) const -> Result<std::vector<std::byte>>;
	[[nodiscard]] auto get_string(Request request) const -> Result<std::string>;

  protected:
	[[nodiscard]] virtual auto perform_download(downloader::Request const& request) const -> downloader::Result;
};
} // namespace downloader::http
