#include "acting_dialog.h"

#include "constants.h"
#include "dark_mode.h"
#include "handlers.h"
#include "wrappers/zmq/poll.h"

#include <algorithm>
#include <vector>

#include <CommCtrl.h>

namespace linkollector::win {

acting_dialog::~acting_dialog() noexcept {
    if (m_worker_thread != nullptr) {
        wrappers::zmq::socket signal_socket(m_ctx,
                                            wrappers::zmq::socket::type::pair);

        if (!signal_socket.connect("inproc://signal")) {
            MessageBoxW(nullptr,
                        L"Failed to connect to the signal socket",
                        L"Error",
                        MB_ICONERROR);
            std::abort();
        }

        if (!signal_socket.blocking_send()) {
            MessageBoxW(nullptr,
                        L"Failed to send to the signal socket",
                        L"Error",
                        MB_ICONERROR);
            std::abort();
        }

        m_worker_thread->join();
    }
}

LRESULT
acting_dialog::show_receiving(HINSTANCE instance,
                              HWND parent,
                              wrappers::zmq::context &ctx) {
    acting_dialog dlg(
        action::receiving, instance, parent, ctx, L"", activity::text, L"");
    return dlg.show_();
}

LRESULT
acting_dialog::show_sending(HINSTANCE instance,
                            HWND parent,
                            wrappers::zmq::context &ctx,
                            std::wstring server,
                            activity activity_,
                            std::wstring message) {
    acting_dialog dlg(action::sending,
                      instance,
                      parent,
                      ctx,
                      std::move(server),
                      activity_,
                      std::move(message));
    return dlg.show_();
}

acting_dialog::acting_dialog(action action_,
                             HINSTANCE instance,
                             HWND parent,
                             wrappers::zmq::context &ctx,
                             std::wstring server,
                             activity activity_,
                             std::wstring message) noexcept
    : m_action(action_), m_instance(instance), m_parent(parent), m_ctx(ctx),
      m_server(std::move(server)), m_activity(activity_),
      m_message(std::move(message)) {}

LRESULT
acting_dialog::show_() noexcept {
    const auto title = m_action == action::receiving
                           ? std::wstring(L"Receiving...")
                           : std::wstring(L"Sending...");

    std::vector<std::byte> buf;
    buf.resize(sizeof(DLGTEMPLATE) + sizeof(WORD) + sizeof(WORD) +
                   (sizeof(wchar_t) * (title.size() + 1)),
               static_cast<std::byte>(0));

    auto *dialog_template = reinterpret_cast<LPDLGTEMPLATE>(buf.data());

    dialog_template->style =
        WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION;
    dialog_template->cdit = 0;
    dialog_template->x = 0;
    dialog_template->y = 0;
    dialog_template->cx = 0;
    dialog_template->cy = 0;

    const auto title_offset =
        sizeof(DLGTEMPLATE) + sizeof(WORD) + sizeof(WORD);
    auto *byte_title_pos = &(buf[title_offset]);
    auto *title_pos = reinterpret_cast<wchar_t *>(byte_title_pos);

    std::copy(std::begin(title), std::end(title), title_pos);

    return DialogBoxIndirectParamW(m_instance,
                                   dialog_template,
                                   m_parent,
                                   acting_dialog::dialog_proc,
                                   reinterpret_cast<LPARAM>(this));
}

INT_PTR CALLBACK acting_dialog::dialog_proc(HWND hwnd,
                                            UINT message_code,
                                            WPARAM w_param,
                                            LPARAM l_param) noexcept {
    const auto get_this = [hwnd]() -> acting_dialog * {
        return reinterpret_cast<acting_dialog *>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    };

    switch (message_code) {
    case WM_INITDIALOG: {
        auto *this_ = reinterpret_cast<acting_dialog *>(l_param);
        SetWindowLongPtrW(
            hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this_));
        this_->m_hwnd = hwnd;
        this_->m_current_dpi =
            static_cast<int>(GetDpiForWindow(this_->m_hwnd));

        enable_dark_mode(this_->m_hwnd, true);
        if (is_dark_mode_enabled()) {
            refresh_non_client_area(this_->m_hwnd);
        }

        LOGFONT log_font;
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                                   sizeof(log_font),
                                   &log_font,
                                   FALSE,
                                   static_cast<UINT>(this_->m_current_dpi));

        this_->m_font = font(log_font);

        RECT main_window_client_rect;
        GetClientRect(hwnd, &main_window_client_rect);

        MapWindowPoints(hwnd,
                        nullptr,
                        reinterpret_cast<POINT *>(&main_window_client_rect),
                        2);

        main_window_client_rect.right =
            main_window_client_rect.left + MulDiv(ACTING_DIALOG_WIDTH_96,
                                                  this_->m_current_dpi,
                                                  USER_DEFAULT_SCREEN_DPI);
        main_window_client_rect.bottom =
            main_window_client_rect.top + MulDiv(ACTING_DIALOG_HEIGHT_96,
                                                 this_->m_current_dpi,
                                                 USER_DEFAULT_SCREEN_DPI);

        AdjustWindowRectExForDpi(&main_window_client_rect,
                                 WS_OVERLAPPEDWINDOW,
                                 FALSE,
                                 0,
                                 static_cast<UINT>(this_->m_current_dpi));

        MoveWindow(
            hwnd,
            main_window_client_rect.left,
            main_window_client_rect.top,
            main_window_client_rect.right - main_window_client_rect.left,
            main_window_client_rect.bottom - main_window_client_rect.top,
            TRUE);

        this_->m_progress_bar =
            CreateWindowExW(0,
                            PROGRESS_CLASS,
                            nullptr,
                            WS_VISIBLE | WS_CHILD | PBS_MARQUEE,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            GetModuleHandleW(nullptr),
                            nullptr);

        SendMessageW(this_->m_progress_bar,
                     PBM_SETMARQUEE,
                     TRUE,
                     PROGRESS_BAR_ANIMATION_MS_96 /
                         (this_->m_current_dpi / USER_DEFAULT_SCREEN_DPI));

        this_->m_button_cancel = CreateWindowExW(
            0,
            WC_BUTTON,
            L"Cancel",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD,
            0,
            0,
            0,
            0,
            hwnd,
            reinterpret_cast<HMENU>(acting_dialog::m_button_cancel_id),
            GetModuleHandleW(nullptr),
            nullptr);

        SendMessageW(this_->m_button_cancel,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(this_->m_font.get()),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(hwnd,
                     WM_SIZE,
                     0,
                     MAKELPARAM(MulDiv(ACTING_DIALOG_WIDTH_96,
                                       this_->m_current_dpi,
                                       USER_DEFAULT_SCREEN_DPI),
                                MulDiv(ACTING_DIALOG_HEIGHT_96,
                                       this_->m_current_dpi,
                                       USER_DEFAULT_SCREEN_DPI)));

        HWND parent = GetParent(hwnd);

        RECT rect_parent;
        GetWindowRect(parent, &rect_parent);

        RECT rect_dialog;
        GetWindowRect(hwnd, &rect_dialog);

        RECT rect_centered;
        CopyRect(&rect_centered, &rect_parent);
        OffsetRect(&rect_dialog, -rect_dialog.left, -rect_dialog.top);
        OffsetRect(&rect_centered, -rect_centered.left, -rect_centered.top);
        OffsetRect(&rect_centered, -rect_dialog.right, -rect_dialog.bottom);

        SetWindowPos(hwnd,
                     nullptr,
                     rect_parent.left + (rect_centered.right / 2),
                     rect_parent.top + (rect_centered.bottom / 2),
                     0,
                     0,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

        this_->m_signal_socket = std::make_unique<wrappers::zmq::socket>(
            this_->m_ctx, wrappers::zmq::socket::type::pair);

        if (!this_->m_signal_socket->bind("inproc://signal")) {
            this_->show_centered_message_box(L"Failed to bind signal socket",
                                             L"Error");
            EndDialog(hwnd, FALSE);
        }

        if (this_->m_action == acting_dialog::action::receiving) {
            this_->m_tcp_socket = std::make_unique<wrappers::zmq::socket>(
                this_->m_ctx, wrappers::zmq::socket::type::rep);

            this_->m_worker_thread = std::make_unique<std::thread>(
                acting_dialog::loop_receive,
                hwnd,
                std::ref(*this_->m_signal_socket),
                std::ref(*this_->m_tcp_socket));
        } else {
            this_->m_tcp_socket = std::make_unique<wrappers::zmq::socket>(
                this_->m_ctx, wrappers::zmq::socket::type::req);

            this_->m_worker_thread = std::make_unique<std::thread>(
                acting_dialog::loop_send,
                hwnd,
                std::ref(*this_->m_signal_socket),
                std::ref(*this_->m_tcp_socket),
                std::ref(this_->m_server),
                this_->m_activity,
                std::ref(this_->m_message));
        }

        return TRUE;
    }

    case WM_SIZE: {
        auto *this_ = get_this();
        if (this_ == nullptr) {
            return 0;
        }

        const auto width = LOWORD(l_param);
        const auto height = HIWORD(l_param);

        const auto progress_bar_width = MulDiv(PROGRESS_BAR_WIDTH_96,
                                               this_->m_current_dpi,
                                               USER_DEFAULT_SCREEN_DPI);
        const auto progress_bar_height = MulDiv(PROGRESS_BAR_HEIGHT_96,
                                                this_->m_current_dpi,
                                                USER_DEFAULT_SCREEN_DPI);

        SetWindowPos(this_->m_progress_bar,
                     nullptr,
                     (width / 2) - (progress_bar_width / 2),
                     (height / 3) - (progress_bar_height / 2),
                     progress_bar_width,
                     progress_bar_height,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        const int button_width_minimum = MulDiv(BUTTON_WIDTH_MINIMUM_96,
                                                this_->m_current_dpi,
                                                USER_DEFAULT_SCREEN_DPI);
        SIZE button_size;
        Button_GetIdealSize(this_->m_button_cancel, &button_size);

        if (button_size.cx < button_width_minimum) {
            button_size.cx = button_width_minimum;
        }

        SetWindowPos(this_->m_button_cancel,
                     nullptr,
                     (width / 2) - (button_size.cx / 2),
                     ((height / 3) * 2) - (button_size.cy / 2),
                     button_size.cx,
                     button_size.cy,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        return 0;
    }

    case WM_GETDPISCALEDSIZE: {
        auto *this_ = get_this();
        if (this_ == nullptr) {
            return TRUE;
        }

        int new_dpi = static_cast<int>(w_param);
        if (new_dpi == 0) {
            return TRUE;
        }

        SIZE *new_size = reinterpret_cast<SIZE *>(l_param);

        const auto scaling_factor =
            static_cast<double>(new_dpi) / this_->m_current_dpi;

        RECT client_area;
        GetClientRect(hwnd, &client_area);

        client_area.right = static_cast<LONG>(
            static_cast<double>(client_area.right) * scaling_factor);
        client_area.bottom = static_cast<LONG>(
            static_cast<double>(client_area.bottom) * scaling_factor);

        AdjustWindowRectExForDpi(&client_area,
                                 WS_OVERLAPPEDWINDOW,
                                 FALSE,
                                 0,
                                 static_cast<UINT>(new_dpi));

        new_size->cx = client_area.right - client_area.left;
        new_size->cy = client_area.bottom - client_area.top;

        this_->m_current_dpi = new_dpi;

        return TRUE;
    }

    case WM_DPICHANGED: {
        auto *this_ = get_this();
        if (this_ == nullptr) {
            return 0;
        }

        auto *new_rect = reinterpret_cast<RECT *>(l_param);

        LOGFONT log_font;
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                                   sizeof(log_font),
                                   &log_font,
                                   FALSE,
                                   static_cast<UINT>(this_->m_current_dpi));

        this_->m_font = font(log_font);

        SendMessageW(this_->m_button_cancel,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(this_->m_font.get()),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(this_->m_progress_bar,
                     PBM_SETMARQUEE,
                     TRUE,
                     PROGRESS_BAR_ANIMATION_MS_96 /
                         (this_->m_current_dpi / USER_DEFAULT_SCREEN_DPI));

        SetWindowPos(hwnd,
                     nullptr,
                     new_rect->left,
                     new_rect->top,
                     new_rect->right - new_rect->left,
                     new_rect->bottom - new_rect->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        return 0;
    }

    case WM_COMMAND: {
        auto *this_ = get_this();
        if (this_ == nullptr) {
            return 0;
        }

        if (LOWORD(w_param) == acting_dialog::m_button_cancel_id) {
            EndDialog(hwnd, TRUE);
        }
        return 0;
    }

    case WM_CLOSE: {
        EndDialog(hwnd, TRUE);
        return 0;
    }

    case acting_dialog::WM_LINKOLLECTOR_ERROR: {
        auto *error_msg_ = reinterpret_cast<std::wstring *>(l_param);
        const std::wstring error_msg = std::move(*error_msg_);
        delete error_msg_;

        auto *this_ = get_this();
        if (this_ == nullptr) {
            MessageBoxW(hwnd, error_msg.c_str(), L"Error", 0);
            EndDialog(hwnd, FALSE);
            return 0;
        }

        this_->show_centered_message_box(error_msg, L"Error");
        EndDialog(hwnd, FALSE);
        return 0;
    }

    case acting_dialog::WM_LINKOLLECTOR_RECEVIED: {
        const auto activity_ = static_cast<activity>(w_param);
        auto *msg_ = reinterpret_cast<std::wstring *>(l_param);
        const std::wstring msg = std::move(*msg_);
        delete msg_;

        switch (activity_) {
        case activity::url: {
            handle_url(msg);
            break;
        }
        case activity::text: {
            handle_text(msg);
            break;
        }
            LINKOLLECTOR_UNREACHABLE;
        }

        EndDialog(hwnd, TRUE);
        return 0;
    }

    case acting_dialog::WM_LINKOLLECTOR_SENT: {
        EndDialog(hwnd, TRUE);
        return 0;
    }
    }

    return FALSE;
}

void acting_dialog::show_centered_message_box(
    const std::wstring &text, const std::wstring &title) noexcept {
    m_current_hook =
        SetWindowsHookExW(WH_CALLWNDPROCRET,
                          acting_dialog::centered_message_box_hook,
                          nullptr,
                          GetCurrentThreadId());
    MessageBoxW(this->m_hwnd, text.c_str(), title.c_str(), MB_ICONERROR);
}

LRESULT CALLBACK acting_dialog::centered_message_box_hook(
    int code, WPARAM w_param, LPARAM l_param) noexcept {
    if (code < 0) {
        return CallNextHookEx(nullptr, code, w_param, l_param);
    }

    if (reinterpret_cast<CWPRETSTRUCT *>(l_param)->message != HCBT_ACTIVATE) {
        return CallNextHookEx(nullptr, code, w_param, l_param);
    }

    HWND message_box = reinterpret_cast<CWPRETSTRUCT *>(l_param)->hwnd;
    HWND parent = GetParent(message_box);

    auto *this_ = reinterpret_cast<acting_dialog *>(
        GetWindowLongPtrW(parent, GWLP_USERDATA));

    RECT rect;
    GetWindowRect(parent, &rect);

    RECT rect_message_box;
    GetWindowRect(message_box, &rect_message_box);

    RECT rect_centered;
    CopyRect(&rect_centered, &rect);
    OffsetRect(
        &rect_message_box, -rect_message_box.left, -rect_message_box.top);
    OffsetRect(&rect_centered, -rect_centered.left, -rect_centered.top);
    OffsetRect(
        &rect_centered, -rect_message_box.right, -rect_message_box.bottom);

    SetWindowPos(message_box,
                 nullptr,
                 rect.left + (rect_centered.right / 2),
                 rect.top + (rect_centered.bottom / 2),
                 0,
                 0,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

    UnhookWindowsHookEx(this_->m_current_hook);
    this_->m_current_hook = nullptr;

    return CallNextHookEx(nullptr, code, w_param, l_param);
}

void acting_dialog::loop_receive(HWND hwnd,
                                 wrappers::zmq::socket &signal_socket,
                                 wrappers::zmq::socket &tcp_socket) noexcept {
    const auto post_error = [hwnd](const wchar_t *error_msg) {
        PostMessageW(hwnd,
                     WM_LINKOLLECTOR_ERROR,
                     0,
                     reinterpret_cast<LPARAM>(new std::wstring(error_msg)));
    };

    if (!tcp_socket.bind("tcp://*:17729")) {
        post_error(L"Failed to bind the TCP responder socket");
        return;
    }

    std::array<wrappers::zmq::poll_target, 2> items = {
        wrappers::zmq::poll_target(signal_socket,
                                   wrappers::zmq::poll_event::in),
        wrappers::zmq::poll_target(tcp_socket, wrappers::zmq::poll_event::in),
    };

    bool break_loop = false;

    while (!break_loop) {
        auto maybe_responses = wrappers::zmq::blocking_poll(items);

        if (!maybe_responses.has_value()) {
            post_error(L"Failure in zmq_poll, killing server...");
            break;
        }

        const auto responses = std::move(*maybe_responses);

        if (responses.empty()) {
            continue;
        }

        for (const auto &response : responses) {
            if (response.response_socket == &signal_socket &&
                response.response_event == wrappers::zmq::poll_event::in) {
                if (!signal_socket.blocking_receive()) {
                    post_error(L"Failed to receive answer from signal socket");
                }
                break_loop = true;
                break;
            }

            if (response.response_socket == &tcp_socket &&
                response.response_event == wrappers::zmq::poll_event::in) {
                using payload_t = std::tuple<
                    wrappers::zmq::socket &,
                    std::optional<std::pair<activity, std::string>> &,
                    bool &,
                    decltype(post_error) &>;

                std::optional<std::pair<activity, std::string>> maybe_data;
                bool did_error = false;

                payload_t payload(
                    tcp_socket, maybe_data, did_error, post_error);

                const auto on_message = [](void *payload_,
                                           gsl::span<std::byte> msg) {
                    auto &[tcp_socket_, maybe_data_, did_error_, post_error_] =
                        *static_cast<payload_t *>(payload_);

                    if (!tcp_socket_.blocking_send()) {
                        post_error_(L"Failed to send data to the TCP "
                                    L"responder socket");
                        did_error_ = true;
                        return;
                    }

                    if (msg.empty()) {
                        return;
                    }

                    maybe_data_ = deserialize(msg);
                };

                if (did_error) {
                    break_loop = true;
                    break;
                }

                if (!tcp_socket.async_receive(static_cast<void *>(&payload),
                                              on_message)) {
                    post_error(L"Failure in zmq_msg_recv, killing server...");
                    break_loop = true;
                    break;
                }

                if (!maybe_data.has_value()) {
                    post_error(L"Could not parse message from client");
                    continue;
                }

                auto data = std::move(*maybe_data);
                const auto activity_ = data.first;
                const auto string_ = std::move(data.second);

                const int wide_char_characters = MultiByteToWideChar(
                    CP_UTF8, 0, string_.c_str(), -1, nullptr, 0);

                auto *out = new std::wstring(
                    static_cast<std::size_t>(wide_char_characters), L'\0');

                MultiByteToWideChar(CP_UTF8,
                                    0,
                                    string_.c_str(),
                                    -1,
                                    out->data(),
                                    wide_char_characters);

                PostMessageW(hwnd,
                             acting_dialog::WM_LINKOLLECTOR_RECEVIED,
                             static_cast<WPARAM>(activity_),
                             reinterpret_cast<LPARAM>(out));
            }
        }
    }
}

void acting_dialog::loop_send(HWND hwnd,
                              wrappers::zmq::socket &signal_socket,
                              wrappers::zmq::socket &tcp_socket,
                              const std::wstring &server,
                              const activity activity_,
                              const std::wstring &message) noexcept {
    const auto post_error = [hwnd](const wchar_t *error_msg) {
        PostMessageW(hwnd,
                     WM_LINKOLLECTOR_ERROR,
                     0,
                     reinterpret_cast<LPARAM>(new std::wstring(error_msg)));
    };

    const int server_utf8_characters =
        WideCharToMultiByte(CP_UTF8,
                            0,
                            server.c_str(),
                            static_cast<int>(server.size()),
                            nullptr,
                            0,
                            nullptr,
                            nullptr);

    std::string server_utf8(static_cast<std::size_t>(server_utf8_characters),
                            '\0');

    WideCharToMultiByte(CP_UTF8,
                        0,
                        server.c_str(),
                        static_cast<int>(server.size()),
                        server_utf8.data(),
                        server_utf8_characters,
                        nullptr,
                        nullptr);

    std::string s = "tcp://" + server_utf8 + ":17729";

    if (!tcp_socket.connect("tcp://" + server_utf8 + ":17729")) {
        post_error(L"Failed to connect the TCP requester socket");
        return;
    }

    const int message_utf8_characters =
        WideCharToMultiByte(CP_UTF8,
                            0,
                            message.c_str(),
                            static_cast<int>(message.size()),
                            nullptr,
                            0,
                            nullptr,
                            nullptr);

    std::string message_utf8(static_cast<std::size_t>(message_utf8_characters),
                             '\0');

    WideCharToMultiByte(CP_UTF8,
                        0,
                        message.c_str(),
                        static_cast<int>(message.size()),
                        message_utf8.data(),
                        message_utf8_characters,
                        nullptr,
                        nullptr);

    std::string data;
    data += activity_to_string(activity_);
    data += activity_delimiter;
    data += message_utf8;

    if (!tcp_socket.blocking_send(
            {static_cast<std::byte *>(static_cast<void *>(data.data())),
             data.size()})) {
        post_error(L"Failed to send data to the TCP requester socket");
        return;
    }

    std::array<wrappers::zmq::poll_target, 2> items = {
        wrappers::zmq::poll_target(signal_socket,
                                   wrappers::zmq::poll_event::in),
        wrappers::zmq::poll_target(tcp_socket, wrappers::zmq::poll_event::in),
    };

    bool break_loop = false;

    while (!break_loop) {
        auto maybe_responses = wrappers::zmq::blocking_poll(items);

        if (!maybe_responses.has_value()) {
            post_error(L"Failure in zmq_poll, killing client...");
            break;
        }

        const auto responses = std::move(*maybe_responses);

        if (responses.empty()) {
            continue;
        }

        for (const auto &response : responses) {
            if (response.response_socket == &signal_socket &&
                response.response_event == wrappers::zmq::poll_event::in) {

                if (!signal_socket.blocking_receive()) {
                    post_error(L"Failed to receive answer from signal socket");
                }
                break_loop = true;
                break;
            }

            if (response.response_socket == &tcp_socket &&
                response.response_event == wrappers::zmq::poll_event::in) {

                if (!tcp_socket.async_receive(nullptr, nullptr)) {
                    post_error(L"Failure in zmq_msg_recv, killing client...");
                }

                break_loop = true;
                break;
            }
        }
    }

    PostMessageW(hwnd, acting_dialog::WM_LINKOLLECTOR_SENT, 0, 0);
}

} // namespace linkollector::win
