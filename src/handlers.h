#pragma once

#include <string>

namespace linkollector::win {

void handle_url(const std::wstring &url) noexcept;

void handle_text(const std::wstring &text) noexcept;

} // namespace linkollector::win
