#pragma once
#include <array>
#include <cstdint>

namespace downloader::http {
/// \brief Wrapper over a response status code.
class Status {
  public:
	/// \brief Response Status Code.
	// NOLINTNEXTLINE(performance-enum-size)
	enum struct Code : std::int64_t {
		Ok = 200,
	};

	/// \brief Category of Response status code.
	enum class Category : std::int8_t {
		Informational,
		Successful,
		Redirection,
		ClientError,
		ServerError,
		Other,
	};

	Status() = default;

	/// \param code: Pass a downloader::Response::code here.
	explicit constexpr Status(std::int64_t const code) : m_code(Code{code}), m_category(get_status_category(m_code)) {}

	[[nodiscard]] constexpr auto get_code() const -> Code { return m_code; }
	[[nodiscard]] constexpr auto get_category() const -> Category { return m_category; }

	[[nodiscard]] constexpr auto is_success() const -> bool { return m_category == Category::Successful; }
	[[nodiscard]] constexpr auto is_error() const -> bool {
		return m_category == Category::ClientError || m_category == Category::ServerError;
	}

	explicit(false) constexpr operator Code() const { return get_code(); }

  private:
	struct CategoryRange {
		static constexpr std::int64_t range_v{100};

		[[nodiscard]] constexpr auto in_range(Status::Code const in) const -> bool {
			return in >= lo && in < Status::Code(std::int64_t(lo) + range_v);
		}

		Status::Category category{};
		Status::Code lo{};
	};

	static constexpr auto category_ranges_v = std::array{
		CategoryRange{.category = Status::Category::Informational, .lo = Status::Code{100}},
		CategoryRange{.category = Status::Category::Successful, .lo = Status::Code{200}},
		CategoryRange{.category = Status::Category::Redirection, .lo = Status::Code{300}},
		CategoryRange{.category = Status::Category::ClientError, .lo = Status::Code{400}},
		CategoryRange{.category = Status::Category::ServerError, .lo = Status::Code{500}},
	};

	static constexpr auto get_status_category(Status::Code const status_code) -> Status::Category {
		for (auto const& range : category_ranges_v) {
			if (range.in_range(status_code)) { return range.category; }
		}
		return Status::Category::Other;
	}

	Code m_code{};
	Category m_category{};
};
} // namespace downloader::http
