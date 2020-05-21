#include "acting_dialog.h"

#include "constants.h"
#include "dark_mode.h"

#include <algorithm>
#include <vector>

#include <CommCtrl.h>

namespace linkollector::win {

LRESULT
acting_dialog::show(HINSTANCE instance,
                    HWND parent,
                    std::wstring_view title,
                    wrappers::zmq::context &ctx) {
    acting_dialog dlg(instance, parent, ctx);
    return dlg.show_(title);
}

acting_dialog::acting_dialog(HINSTANCE instance,
                             HWND parent,
                             wrappers::zmq::context &ctx) noexcept
    : m_instance(instance), m_parent(parent), m_ctx(ctx) {}

LRESULT
acting_dialog::show_(std::wstring_view title) {
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

        this_->m_button_cancel =
            CreateWindowExW(0,
                            WC_BUTTON,
                            L"Cancel",
                            WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            reinterpret_cast<HMENU>(this_->m_button_cancel_id),
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
            this_->show_centered_message_box(L"Test", L"");
            //            EndDialog(hwnd, TRUE);
        }
        return 0;
    }

    case WM_CLOSE: {
        EndDialog(hwnd, TRUE);
        return 0;
    }
    }

    return FALSE;
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

void acting_dialog::show_centered_message_box(
    const std::wstring &text, const std::wstring &title) noexcept {
    m_current_hook =
        SetWindowsHookExW(WH_CALLWNDPROCRET,
                          acting_dialog::centered_message_box_hook,
                          nullptr,
                          GetCurrentThreadId());
    MessageBoxW(this->m_hwnd, text.c_str(), title.c_str(), 0);
}

} // namespace linkollector::win
