#pragma once

#include "macros.h"

#include <gsl/span>

#include <array>
#include <optional>
#include <string>

namespace linkollector::win {

enum class activity { url, text };

constexpr std::string_view activity_delimiter = "\uedfd";

constexpr std::array<std::byte, activity_delimiter.size()>
    activity_delimiter_bin = []() {
        std::array<std::byte, activity_delimiter.size()> delim = {};
        for (std::size_t i = 0; i < activity_delimiter.size(); ++i) {
            delim.at(i) = static_cast<std::byte>(activity_delimiter[i]);
        }
        return delim;
    }();

[[nodiscard]] std::string activity_to_string(activity activity_) noexcept;

[[nodiscard]] std::wstring activity_to_wstring(activity activity_) noexcept;

[[nodiscard]] std::optional<activity>
activity_from_string(std::string_view activity_) noexcept;

std::optional<std::pair<activity, std::string>>
deserialize(gsl::span<std::byte> msg) noexcept;

} // namespace linkollector::win
