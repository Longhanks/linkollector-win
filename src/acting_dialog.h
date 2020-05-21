#pragma once

#include "font.h"

#include <Windows.h>

#include <string>

namespace linkollector::win {

class acting_dialog {

public:
    acting_dialog(const acting_dialog &other) = delete;
    acting_dialog &operator=(const acting_dialog &other) = delete;
    acting_dialog(acting_dialog &&other) noexcept = default;
    acting_dialog &operator=(acting_dialog &&other) noexcept = default;
    ~acting_dialog() noexcept = default;

    static LRESULT
    show(HINSTANCE instance, HWND parent, std::wstring_view title);

private:
    constexpr static const UINT WM_LINKOLLECTOR_RECEIVED = WM_APP + 1;

    explicit acting_dialog(HINSTANCE instance, HWND parent) noexcept;

    LRESULT show_(std::wstring_view title);

    static INT_PTR CALLBACK dialog_proc(HWND hwnd,
                                        UINT message_code,
                                        WPARAM w_param,
                                        LPARAM l_param) noexcept;

    HINSTANCE m_instance = nullptr;
    HWND m_hwnd = nullptr;
    HWND m_parent = nullptr;
    int m_current_dpi = -1;
    font m_font;

    HWND m_button_cancel = nullptr;
    constexpr static const int m_button_cancel_id = 100;
    HWND m_progress_bar = nullptr;
};

} // namespace linkollector::win
