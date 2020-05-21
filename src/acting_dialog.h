#pragma once

#include "font.h"
#include "wrappers/zmq/context.h"

#include <Windows.h>

#include <string>

namespace linkollector::win {

class acting_dialog {

public:
    acting_dialog(const acting_dialog &other) = delete;
    acting_dialog &operator=(const acting_dialog &other) = delete;
    acting_dialog(acting_dialog &&other) noexcept = delete;
    acting_dialog &operator=(acting_dialog &&other) noexcept = delete;
    ~acting_dialog() noexcept = default;

    static LRESULT show(HINSTANCE instance,
                        HWND parent,
                        std::wstring_view title,
                        wrappers::zmq::context &ctx);

private:
    constexpr static const UINT WM_LINKOLLECTOR_RECEIVED = WM_APP + 1;

    explicit acting_dialog(HINSTANCE instance,
                           HWND parent,
                           wrappers::zmq::context &ctx) noexcept;

    LRESULT show_(std::wstring_view title);

    static INT_PTR CALLBACK dialog_proc(HWND hwnd,
                                        UINT message_code,
                                        WPARAM w_param,
                                        LPARAM l_param) noexcept;

    void show_centered_message_box(const std::wstring &text,
                                   const std::wstring &title) noexcept;

    static LRESULT CALLBACK centered_message_box_hook(int code,
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

    wrappers::zmq::context &m_ctx;

    HHOOK m_current_hook = nullptr;
};

} // namespace linkollector::win
