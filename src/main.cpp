#include "constants.h"
#include "line.h"

#include <cstdlib>
#include <string>

#include <Windows.h>

#include <CommCtrl.h>

static HFONT system_font;
static HWND button_receive;
static HWND separator_line_left;
static HWND label_or;
static HWND separator_line_right;

static void autolayout(HWND hwnd, UINT dpi) noexcept {
    const auto dpiscaled = [&dpi](int val) -> int {
        return MulDiv(val, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI);
    };

    RECT main_window_rect;
    GetClientRect(hwnd, &main_window_rect);

    HDC hdc = GetDC(hwnd);

    const auto label_or_string_size = GetWindowTextLengthW(label_or);
    std::wstring label_or_string;
    label_or_string.resize(static_cast<std::size_t>(label_or_string_size));
    GetWindowTextW(label_or, label_or_string.data(), label_or_string_size);

    SIZE size_label_or;
    GetTextExtentPoint32W(
        hdc, label_or_string.data(), label_or_string_size, &size_label_or);
    size_label_or.cy = dpiscaled(size_label_or.cy);
    size_label_or.cx = dpiscaled(size_label_or.cx);

    SetWindowPos(label_or,
                 nullptr,
                 ((main_window_rect.right / 2) - (size_label_or.cx / 2)),
                 ((main_window_rect.bottom / 2) - (size_label_or.cy / 2)),
                 size_label_or.cx,
                 size_label_or.cy,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(label_or, nullptr, FALSE);

    SetWindowPos(separator_line_left,
                 nullptr,
                 /* x: */ dpiscaled(LINE_PADDING),
                 /* y: */ (main_window_rect.bottom / 2),
                 /* width: */
                 ((main_window_rect.right / 2) - (size_label_or.cx / 2) -
                  (dpiscaled(LINE_PADDING) * 2)),
                 /* height: */ dpiscaled(LINE_HEIGHT),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    SetWindowPos(separator_line_right,
                 nullptr,
                 /* x: */ (main_window_rect.right / 2) +
                     (size_label_or.cx / 2) + dpiscaled(LINE_PADDING),
                 /* y: */ (main_window_rect.bottom / 2),
                 /* width: */
                 ((main_window_rect.right / 2) - (size_label_or.cx / 2) -
                  (dpiscaled(LINE_PADDING) * 2)),
                 /* height: */ dpiscaled(LINE_HEIGHT),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    SIZE button_receive_size;
    Button_GetIdealSize(button_receive, &button_receive_size);

    SetWindowPos(
        button_receive,
        nullptr,
        /* x: */ ((main_window_rect.right / 2) - (button_receive_size.cx / 2)),
        /* y: */
        ((main_window_rect.bottom / 4) - (button_receive_size.cy / 2)),
        /* width: */ button_receive_size.cx,
        /* height: */ button_receive_size.cy,
        SWP_NOZORDER | SWP_NOACTIVATE);

    ReleaseDC(hwnd, hdc);
}

static LRESULT CALLBACK main_window_f(HWND hwnd,
                                      UINT message_code,
                                      WPARAM w_param,
                                      LPARAM l_param) noexcept {

    switch (message_code) {
    case WM_CREATE: {
        UINT dpi = GetDpiForWindow(hwnd);
        RECT rect_window;
        GetWindowRect(hwnd, &rect_window);
        rect_window.right =
            rect_window.left +
            MulDiv(WINDOW_WIDTH_96, dpi, USER_DEFAULT_SCREEN_DPI);
        rect_window.bottom =
            rect_window.top +
            MulDiv(WINDOW_HEIGHT_96, dpi, USER_DEFAULT_SCREEN_DPI);
        SetWindowPos(hwnd,
                     nullptr,
                     rect_window.right,
                     rect_window.top,
                     rect_window.right - rect_window.left,
                     rect_window.bottom - rect_window.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        LOGFONT log_font;
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                                   sizeof(log_font),
                                   &log_font,
                                   FALSE,
                                   GetDpiForWindow(hwnd));

        system_font = CreateFontIndirectW(&log_font);

        button_receive =
            CreateWindowExW(0,
                            L"BUTTON",
                            L"Receive...",
                            WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        SendMessageW(button_receive,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        label_or =
            CreateWindowExW(0,
                            L"STATIC",
                            L"Or",
                            WS_VISIBLE | WS_CHILD | SS_CENTER,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        SendMessageW(label_or,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        separator_line_left =
            CreateWindowExW(0,
                            L"STATIC",
                            nullptr,
                            WS_VISIBLE | WS_CHILD,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        separator_line_right =
            CreateWindowExW(0,
                            L"STATIC",
                            nullptr,
                            WS_VISIBLE | WS_CHILD,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        SetWindowSubclass(separator_line_left, line_window_f, 0, 0);
        SetWindowSubclass(separator_line_right, line_window_f, 0, 0);

        return 0;
    }

    case WM_SIZE: {
        autolayout(hwnd, GetDpiForWindow(hwnd));
        return DefWindowProcW(hwnd, message_code, w_param, l_param);
    }

    case WM_DPICHANGED: {
        auto *hwnd_new_rect = reinterpret_cast<RECT *>(l_param);

        LOGFONT log_font;
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                                   sizeof(log_font),
                                   &log_font,
                                   FALSE,
                                   HIWORD(w_param));

        DeleteObject(system_font);
        system_font = CreateFontIndirectW(&log_font);

        SendMessageW(button_receive,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(label_or,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SetWindowPos(hwnd,
                     nullptr,
                     hwnd_new_rect->left,
                     hwnd_new_rect->top,
                     hwnd_new_rect->right - hwnd_new_rect->left,
                     hwnd_new_rect->bottom - hwnd_new_rect->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        autolayout(hwnd, HIWORD(w_param));
        return 0;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }

    default: {
        return DefWindowProcW(hwnd, message_code, w_param, l_param);
    }
    }
}

int WINAPI wWinMain(HINSTANCE hInstance,
                    [[maybe_unused]] HINSTANCE hPrevInstance,
                    [[maybe_unused]] PWSTR pCmdLine,
                    int nCmdShow) {
    if (SetProcessDpiAwarenessContext(
            DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != TRUE) {
        MessageBoxW(nullptr, L"Failed to set DPI awareness", nullptr, 0);
        return EXIT_FAILURE;
    }

    INITCOMMONCONTROLSEX init_common_controls_ex;
    init_common_controls_ex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    init_common_controls_ex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    if (InitCommonControlsEx(&init_common_controls_ex) != TRUE) {
        MessageBoxW(
            nullptr, L"Failed to initialize common controls", nullptr, 0);
        return EXIT_FAILURE;
    }

    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = main_window_f;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0,
                                MAIN_WINDOW_CLASS_NAME,
                                L"Linkollector",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                WINDOW_WIDTH_96,
                                WINDOW_HEIGHT_96,
                                HWND_DESKTOP,
                                nullptr,
                                hInstance,
                                nullptr);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(system_font);
}
