#include "acting_dialog.h"

#include "constants.h"

#include <algorithm>
#include <vector>

#include <CommCtrl.h>

namespace linkollector::win {

constexpr static const int button_cancel_id = 100;

static HWND button_cancel = nullptr;
static HWND progress_bar = nullptr;
static HFONT font = nullptr;
static int current_dpi = -1;

static INT_PTR CALLBACK dialog_proc(HWND hwnd,
                                    UINT message_code,
                                    WPARAM w_param,
                                    LPARAM l_param) {
    switch (message_code) {
    case WM_INITDIALOG: {
        current_dpi = static_cast<int>(GetDpiForWindow(hwnd));
        RECT main_window_client_rect;
        GetClientRect(hwnd, &main_window_client_rect);

        MapWindowPoints(hwnd,
                        nullptr,
                        reinterpret_cast<POINT *>(&main_window_client_rect),
                        2);

        main_window_client_rect.right =
            main_window_client_rect.left + MulDiv(ACTING_DIALOG_WIDTH_96,
                                                  current_dpi,
                                                  USER_DEFAULT_SCREEN_DPI);
        main_window_client_rect.bottom =
            main_window_client_rect.top + MulDiv(ACTING_DIALOG_HEIGHT_96,
                                                 current_dpi,
                                                 USER_DEFAULT_SCREEN_DPI);

        AdjustWindowRectExForDpi(&main_window_client_rect,
                                 WS_OVERLAPPEDWINDOW,
                                 FALSE,
                                 0,
                                 static_cast<UINT>(current_dpi));

        MoveWindow(
            hwnd,
            main_window_client_rect.left,
            main_window_client_rect.top,
            main_window_client_rect.right - main_window_client_rect.left,
            main_window_client_rect.bottom - main_window_client_rect.top,
            TRUE);

        LOGFONT log_font;
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                                   sizeof(log_font),
                                   &log_font,
                                   FALSE,
                                   static_cast<UINT>(current_dpi));
        font = CreateFontIndirectW(&log_font);

        progress_bar = CreateWindowExW(0,
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

        SendMessageW(progress_bar,
                     PBM_SETMARQUEE,
                     TRUE,
                     PROGRESS_BAR_ANIMATION_MS_96 /
                         (current_dpi / USER_DEFAULT_SCREEN_DPI));

        button_cancel =
            CreateWindowExW(0,
                            WC_BUTTON,
                            L"Cancel",
                            WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            reinterpret_cast<HMENU>(button_cancel_id),
                            GetModuleHandleW(nullptr),
                            nullptr);

        SendMessageW(button_cancel,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(font),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(hwnd,
                     WM_SIZE,
                     0,
                     MAKELPARAM(MulDiv(ACTING_DIALOG_WIDTH_96,
                                       current_dpi,
                                       USER_DEFAULT_SCREEN_DPI),
                                MulDiv(ACTING_DIALOG_HEIGHT_96,
                                       current_dpi,
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
        const int width = LOWORD(l_param);
        const int height = HIWORD(l_param);

        const auto progress_bar_width = MulDiv(
            PROGRESS_BAR_WIDTH_96, current_dpi, USER_DEFAULT_SCREEN_DPI);
        const auto progress_bar_height = MulDiv(
            PROGRESS_BAR_HEIGHT_96, current_dpi, USER_DEFAULT_SCREEN_DPI);

        SetWindowPos(progress_bar,
                     nullptr,
                     (width / 2) - (progress_bar_width / 2),
                     (height / 3) - (progress_bar_height / 2),
                     progress_bar_width,
                     progress_bar_height,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        const int button_width_minimum = MulDiv(
            BUTTON_WIDTH_MINIMUM_96, current_dpi, USER_DEFAULT_SCREEN_DPI);
        SIZE button_size;
        Button_GetIdealSize(button_cancel, &button_size);

        if (button_size.cx < button_width_minimum) {
            button_size.cx = button_width_minimum;
        }

        SetWindowPos(button_cancel,
                     nullptr,
                     (width / 2) - (button_size.cx / 2),
                     ((height / 3) * 2) - (button_size.cy / 2),
                     button_size.cx,
                     button_size.cy,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        return 0;
    }

    case WM_GETDPISCALEDSIZE: {
        int new_dpi = static_cast<int>(w_param);
        if (new_dpi == 0) {
            return TRUE;
        }

        SIZE *new_size = reinterpret_cast<SIZE *>(l_param);

        const auto scaling_factor = static_cast<double>(new_dpi) / current_dpi;

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

        current_dpi = new_dpi;

        return TRUE;
    }

    case WM_DPICHANGED: {
        auto *new_rect = reinterpret_cast<RECT *>(l_param);

        LOGFONT log_font;
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                                   sizeof(log_font),
                                   &log_font,
                                   FALSE,
                                   static_cast<UINT>(current_dpi));

        if (font != nullptr) {
            DeleteObject(font);
        }

        font = CreateFontIndirectW(&log_font);

        SendMessageW(button_cancel,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(font),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(progress_bar,
                     PBM_SETMARQUEE,
                     TRUE,
                     PROGRESS_BAR_ANIMATION_MS_96 /
                         (current_dpi / USER_DEFAULT_SCREEN_DPI));

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
        if (LOWORD(w_param) == IDOK || LOWORD(w_param) == button_cancel_id) {
            EndDialog(hwnd, TRUE);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE: {
        EndDialog(hwnd, TRUE);
        return TRUE;
    }
    }

    return FALSE;
}

LRESULT
show_acting_dialog(HINSTANCE instance, HWND parent, std::wstring_view title) {
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

    const auto return_value =
        DialogBoxIndirectParamW(instance,
                                reinterpret_cast<LPDLGTEMPLATE>(buf.data()),
                                parent,
                                dialog_proc,
                                0L);

    DeleteObject(font);
    font = nullptr;

    return return_value;
}

} // namespace linkollector::win
