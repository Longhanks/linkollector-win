#pragma once

#include "font.h"
#include "wrappers/zmq/context.h"
#include "wrappers/zmq/socket.h"

#include <Windows.h>

#include <memory>
#include <string>
#include <thread>

namespace linkollector::win {

class acting_dialog {

public:
    enum class action { receiving, sending };

    acting_dialog(const acting_dialog &other) = delete;
    acting_dialog &operator=(const acting_dialog &other) = delete;
    acting_dialog(acting_dialog &&other) noexcept = delete;
    acting_dialog &operator=(acting_dialog &&other) noexcept = delete;
    ~acting_dialog() noexcept;

    static LRESULT show(HINSTANCE instance,
                        HWND parent,
                        action action_,
                        wrappers::zmq::context &ctx);

private:
    // WPARAM: 0
    // LPARAM: std::wstring *
    constexpr static const UINT WM_LINKOLLECTOR_ERROR = WM_APP + 1;

    // WPARAM: activity
    // LPARAM: std::wstring *
    constexpr static const UINT WM_LINKOLLECTOR_RECEVIED = WM_APP + 2;

    explicit acting_dialog(action action_,
                           HINSTANCE instance,
                           HWND parent,
                           wrappers::zmq::context &ctx) noexcept;

    LRESULT show_() noexcept;

    static INT_PTR CALLBACK dialog_proc(HWND hwnd,
                                        UINT message_code,
                                        WPARAM w_param,
                                        LPARAM l_param) noexcept;

    void show_centered_message_box(const std::wstring &text,
                                   const std::wstring &title) noexcept;

    static LRESULT CALLBACK centered_message_box_hook(int code,
                                                      WPARAM w_param,
                                                      LPARAM l_param) noexcept;

    static void loop_receive(HWND hwnd,
                             wrappers::zmq::socket &signal_socket,
                             wrappers::zmq::socket &tcp_socket) noexcept;

    action m_action;

    HINSTANCE m_instance = nullptr;
    HWND m_hwnd = nullptr;
    HWND m_parent = nullptr;
    int m_current_dpi = -1;
    font m_font;

    HWND m_button_cancel = nullptr;
    constexpr static const int m_button_cancel_id = 100;
    HWND m_progress_bar = nullptr;

    wrappers::zmq::context &m_ctx;
    std::unique_ptr<wrappers::zmq::socket> m_signal_socket = nullptr;
    std::unique_ptr<wrappers::zmq::socket> m_tcp_socket = nullptr;
    std::unique_ptr<std::thread> m_worker_thread = nullptr;

    HHOOK m_current_hook = nullptr;
};

} // namespace linkollector::win
