#pragma once

#include <Windows.h>

namespace linkollector::win {

class font {

public:
    explicit font() = default;
    explicit font(LOGFONTW &log_font) noexcept;
    font(const font &other) = delete;
    font &operator=(const font &other) = delete;
    font(font &&other) noexcept;
    font &operator=(font &&other) noexcept;
    ~font() noexcept;

    [[nodiscard]] HFONT get() noexcept;

private:
    HFONT m_font = nullptr;
};

} // namespace linkollector::win
