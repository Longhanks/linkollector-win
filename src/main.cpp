#include "constants.h"
#include "line.h"
#include "textfield.h"

#include <cstdlib>
#include <memory>
#include <string>

#include <Windows.h>

#include <CommCtrl.h>

static HFONT system_font = nullptr;
static HWND button_receive = nullptr;
static HWND separator_line_left = nullptr;
static HWND label_or = nullptr;
static HWND separator_line_right = nullptr;
static HWND label_to_device = nullptr;
static HWND text_field_to_device = nullptr;
static HWND label_message_content = nullptr;
static HWND text_field_message_content = nullptr;
static int current_dpi = -1;

static void autolayout(HWND hwnd, int width, int height, int dpi) noexcept {
    const auto dpiscaled = [&dpi](int val) -> int {
        return MulDiv(val, dpi, USER_DEFAULT_SCREEN_DPI);
    };

    HDC hdc = GetDC(hwnd);

    const auto label_or_string_size = GetWindowTextLengthW(label_or);
    std::wstring label_or_string;
    label_or_string.resize(static_cast<std::size_t>(label_or_string_size));
    GetWindowTextW(label_or, label_or_string.data(), label_or_string_size);

    SIZE size_label_or;
    GetTextExtentPoint32W(
        hdc, label_or_string.data(), label_or_string_size, &size_label_or);
    const auto width_label_or = dpiscaled(size_label_or.cx);
    const auto height_label_or = dpiscaled(size_label_or.cy);

    SetWindowPos(label_or,
                 nullptr,
                 ((width / 2) - (width_label_or / 2)),
                 ((height / 2) - (height_label_or / 2)),
                 width_label_or,
                 height_label_or,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(label_or, nullptr, FALSE);

    SetWindowPos(
        separator_line_left,
        nullptr,
        /* x: */ dpiscaled(LINE_PADDING),
        /* y: */ (height / 2),
        /* width: */
        ((width / 2) - (width_label_or / 2) - (dpiscaled(LINE_PADDING) * 2)),
        /* height: */ dpiscaled(LINE_HEIGHT),
        SWP_NOZORDER | SWP_NOACTIVATE);

    SetWindowPos(
        separator_line_right,
        nullptr,
        /* x: */ (width / 2) + (width_label_or / 2) + dpiscaled(LINE_PADDING),
        /* y: */ (height / 2),
        /* width: */
        ((width / 2) - (width_label_or / 2) - (dpiscaled(LINE_PADDING) * 2)),
        /* height: */ dpiscaled(LINE_HEIGHT),
        SWP_NOZORDER | SWP_NOACTIVATE);

    SIZE button_receive_size;
    Button_GetIdealSize(button_receive, &button_receive_size);

    const int button_width_minimum = dpiscaled(BUTTON_WIDTH_MINIMUM_96);

    if (button_receive_size.cx < button_width_minimum) {
        button_receive_size.cx = button_width_minimum;
    }

    SetWindowPos(button_receive,
                 nullptr,
                 /* x: */ ((width / 2) - (button_receive_size.cx / 2)),
                 /* y: */
                 ((height / 4) - (button_receive_size.cy / 2)),
                 /* width: */ button_receive_size.cx,
                 /* height: */ button_receive_size.cy,
                 SWP_NOZORDER | SWP_NOACTIVATE);

    const auto label_to_device_string_size =
        GetWindowTextLengthW(label_to_device);
    std::wstring label_to_device_string;
    label_to_device_string.resize(
        static_cast<std::size_t>(label_to_device_string_size));
    GetWindowTextW(label_to_device,
                   label_to_device_string.data(),
                   label_to_device_string_size);

    SIZE size_label_to_device;
    GetTextExtentPoint32W(hdc,
                          label_to_device_string.data(),
                          label_to_device_string_size,
                          &size_label_to_device);

    const auto label_to_device_y =
        (height / 2) + (height_label_or / 2) + dpiscaled(CONTENT_PADDING);

    SetWindowPos(label_to_device,
                 nullptr,
                 /* x: */ dpiscaled(CONTENT_PADDING),
                 /* y: */ label_to_device_y,
                 /* width: */ dpiscaled(size_label_to_device.cx),
                 /* height: */ dpiscaled(size_label_to_device.cy),
                 SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(label_to_device, nullptr, FALSE);

    const auto text_field_to_device_y =
        label_to_device_y + dpiscaled(size_label_to_device.cy) +
        dpiscaled(LABEL_TO_TEXT_FIELD_PADDING_96);

    SetWindowPos(text_field_to_device,
                 nullptr,
                 /* x: */ dpiscaled(CONTENT_PADDING),
                 /* y: */ text_field_to_device_y,
                 /* width: */ (width / 2) - (2 * dpiscaled(CONTENT_PADDING)),
                 /* height: */ dpiscaled(TEXT_FIELD_SINGLE_LINE_HEIGHT_96),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    const auto label_message_content_string_size =
        GetWindowTextLengthW(label_message_content);
    std::wstring label_message_content_string;
    label_message_content_string.resize(
        static_cast<std::size_t>(label_message_content_string_size));
    GetWindowTextW(label_message_content,
                   label_message_content_string.data(),
                   label_message_content_string_size);

    SIZE size_label_message_content;
    GetTextExtentPoint32W(hdc,
                          label_message_content_string.data(),
                          label_message_content_string_size,
                          &size_label_message_content);

    const auto label_message_content_y =
        text_field_to_device_y + dpiscaled(TEXT_FIELD_SINGLE_LINE_HEIGHT_96) +
        dpiscaled(CONTENT_PADDING);

    SetWindowPos(label_message_content,
                 nullptr,
                 /* x: */ dpiscaled(CONTENT_PADDING),
                 /* y: */ label_message_content_y,
                 /* width: */ dpiscaled(size_label_message_content.cx),
                 /* height: */ dpiscaled(size_label_message_content.cy),
                 SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(label_message_content, nullptr, FALSE);

    const auto text_field_message_content_y =
        label_message_content_y + dpiscaled(size_label_message_content.cy) +
        dpiscaled(LABEL_TO_TEXT_FIELD_PADDING_96);

    SetWindowPos(text_field_message_content,
                 nullptr,
                 /* x: */ dpiscaled(CONTENT_PADDING),
                 /* y: */ text_field_message_content_y,
                 /* width: */ (width / 2) - (2 * dpiscaled(CONTENT_PADDING)),
                 /* height: */ height - text_field_message_content_y -
                     dpiscaled(CONTENT_PADDING),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    ReleaseDC(hwnd, hdc);
}

static LRESULT CALLBACK main_window_f(HWND hwnd,
                                      UINT message_code,
                                      WPARAM w_param,
                                      LPARAM l_param) noexcept {
    switch (message_code) {
    case WM_CREATE: {
        current_dpi = static_cast<int>(GetDpiForWindow(hwnd));
        LOGFONT log_font;
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                                   sizeof(log_font),
                                   &log_font,
                                   FALSE,
                                   static_cast<UINT>(current_dpi));

        system_font = CreateFontIndirectW(&log_font);

        button_receive =
            CreateWindowExW(0,
                            WC_BUTTON,
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
                            WC_STATIC,
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
                            WC_STATIC,
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
                            WC_STATIC,
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

        label_to_device =
            CreateWindowExW(0,
                            WC_STATIC,
                            L"To:",
                            WS_VISIBLE | WS_CHILD | SS_LEFT,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        SendMessageW(label_to_device,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        text_field_to_device =
            CreateWindowExW(WS_EX_CLIENTEDGE,
                            WC_EDIT,
                            nullptr,
                            WS_VISIBLE | WS_CHILD | ES_WANTRETURN |
                                ES_MULTILINE | ES_AUTOHSCROLL,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        SendMessageW(text_field_to_device,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SetWindowSubclass(text_field_to_device,
                          text_field_window_f,
                          ID_TEXT_FIELD_TO_DEVICE,
                          0);

        label_message_content =
            CreateWindowExW(0,
                            WC_STATIC,
                            L"Content:",
                            WS_VISIBLE | WS_CHILD | SS_LEFT,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        SendMessageW(label_message_content,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        text_field_message_content =
            CreateWindowExW(WS_EX_CLIENTEDGE,
                            WC_EDIT,
                            nullptr,
                            WS_VISIBLE | WS_CHILD | ES_WANTRETURN |
                                ES_MULTILINE | ES_AUTOVSCROLL,
                            0,
                            0,
                            0,
                            0,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        SendMessageW(text_field_message_content,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SetWindowSubclass(text_field_message_content,
                          text_field_window_f,
                          ID_TEXT_FIELD_MESSAGE_CONTENT,
                          0);

        auto *create_struct = reinterpret_cast<CREATESTRUCTW *>(l_param);

        const int cmd_show =
            *static_cast<int *>(create_struct->lpCreateParams);

        // 1. GetClientRect returns (0, 0) as origin and the client area width
        // and height.
        RECT main_window_client_rect;
        GetClientRect(hwnd, &main_window_client_rect);

        // 2. MapWindowPoints translates the origin to absolute screen
        // coordinates.
        MapWindowPoints(hwnd,
                        nullptr,
                        reinterpret_cast<POINT *>(&main_window_client_rect),
                        2);

        // 3. The desired width and height are added to the origin coordinates.
        main_window_client_rect.right =
            main_window_client_rect.left +
            MulDiv(WINDOW_WIDTH_96, current_dpi, USER_DEFAULT_SCREEN_DPI);
        main_window_client_rect.bottom =
            main_window_client_rect.top +
            MulDiv(WINDOW_HEIGHT_96, current_dpi, USER_DEFAULT_SCREEN_DPI);

        // 4. The absolute coordinates are adjusted for the non-client area.
        AdjustWindowRectExForDpi(&main_window_client_rect,
                                 WS_OVERLAPPEDWINDOW,
                                 FALSE,
                                 0,
                                 static_cast<UINT>(current_dpi));

        // 5. The absolute coordinates are used for the origin, and the
        // window's width and height is calculated via the right and bottom
        // coordinates.
        MoveWindow(
            hwnd,
            main_window_client_rect.left,
            main_window_client_rect.top,
            main_window_client_rect.right - main_window_client_rect.left,
            main_window_client_rect.bottom - main_window_client_rect.top,
            TRUE);

        ShowWindow(hwnd, cmd_show);

        return 0;
    }

    case WM_SIZE: {
        const auto width = LOWORD(l_param);
        const auto height = HIWORD(l_param);
        autolayout(
            hwnd, width, height, static_cast<int>(GetDpiForWindow(hwnd)));
        return DefWindowProcW(hwnd, message_code, w_param, l_param);
    }

    case WM_GETDPISCALEDSIZE: {
        const auto previous_dpi = current_dpi;
        current_dpi = static_cast<int>(w_param);
        const auto scaling_factor =
            static_cast<double>(current_dpi) / previous_dpi;

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
                                 static_cast<UINT>(current_dpi));

        SIZE *new_size = reinterpret_cast<SIZE *>(l_param);
        new_size->cx = client_area.right - client_area.left;
        new_size->cy = client_area.bottom - client_area.top;

        return TRUE;
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

        SendMessageW(label_to_device,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(text_field_to_device,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(label_message_content,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(text_field_message_content,
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
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    wc.hIconSm = nullptr;
    RegisterClassExW(&wc);

    auto cmd_show = std::make_unique<int>(nCmdShow);

    CreateWindowExW(0,
                    MAIN_WINDOW_CLASS_NAME,
                    L"Linkollector",
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    nullptr,
                    nullptr,
                    hInstance,
                    static_cast<void *>(cmd_show.get()));

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(system_font);

    return EXIT_SUCCESS;
}
