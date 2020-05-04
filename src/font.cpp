#include "font.h"

namespace linkollector::win {

font::font(LOGFONTW &log_font) noexcept {
    m_font = CreateFontIndirectW(&log_font);
}

font::font(font &&other) noexcept {
    m_font = other.m_font;
    other.m_font = nullptr;
}

font &font::operator=(font &&other) noexcept {
    if (this != &other) {
        m_font = other.m_font;
        other.m_font = nullptr;
    }

    return *this;
}

font::~font() noexcept {
    if (m_font != nullptr) {
        DeleteObject(m_font);
        m_font = nullptr;
    }
}

HFONT font::get() noexcept {
    return m_font;
}

} // namespace linkollector::win
