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

static LRESULT CALLBACK main_window_f(HWND hwnd,
                                      UINT message_code,
                                      WPARAM w_param,
                                      LPARAM l_param) noexcept {

    switch (message_code) {
    case WM_CREATE: {
        NONCLIENTMETRICS metrics;
        metrics.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfoW(
            SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
        system_font = CreateFontIndirectW(&metrics.lfMessageFont);

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

        RECT main_window_rect;
        GetClientRect(hwnd, &main_window_rect);

        HDC hdc = GetDC(hwnd);

        const auto label_or_string_size = GetWindowTextLengthW(label_or);
        std::wstring label_or_string;
        label_or_string.resize(static_cast<std::size_t>(label_or_string_size));
        GetWindowTextW(label_or, label_or_string.data(), label_or_string_size);

        SIZE size_or;
        GetTextExtentPoint32W(
            hdc, label_or_string.data(), label_or_string_size, &size_or);

        SetWindowPos(label_or,
                     HWND_TOP,
                     ((main_window_rect.right / 2) - (size_or.cx / 2)),
                     ((main_window_rect.bottom / 2) - (size_or.cy / 2)),
                     size_or.cx,
                     size_or.cy,
                     0);

        SetWindowPos(separator_line_left,
                     HWND_TOP,
                     LINE_PADDING,
                     (main_window_rect.bottom / 2),
                     ((main_window_rect.right / 2) - (size_or.cx / 2) -
                      (static_cast<LONG>(LINE_PADDING) * 2)),
                     LINE_HEIGHT,
                     0);

        SetWindowPos(separator_line_right,
                     HWND_TOP,
                     (main_window_rect.right / 2) + (size_or.cx / 2) +
                         static_cast<LONG>(LINE_PADDING),
                     (main_window_rect.bottom / 2),
                     ((main_window_rect.right / 2) - (size_or.cx / 2) -
                      (static_cast<LONG>(LINE_PADDING) * 2)),
                     LINE_HEIGHT,
                     0);

        SIZE button_receive_size;
        Button_GetIdealSize(button_receive, &button_receive_size);

        SetWindowPos(
            button_receive,
            HWND_TOP,
            ((main_window_rect.right / 2) - (button_receive_size.cx / 2)),
            ((main_window_rect.bottom / 4) - (button_receive_size.cy / 2)),
            button_receive_size.cx,
            button_receive_size.cy,
            0);

        ReleaseDC(hwnd, hdc);

        return DefWindowProcW(hwnd, message_code, w_param, l_param);
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
                                320,
                                240,
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
