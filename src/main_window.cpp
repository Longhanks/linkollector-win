#include "main_window.h"

#include "constants.h"
#include "dark_mode.h"
#include "line.h"
#include "text_field.h"

#include <algorithm>
#include <array>

#include <CommCtrl.h>
#include <windowsx.h>

namespace linkollector::win {

main_window::main_window(HINSTANCE instance, int cmd_show) noexcept
    : m_instance(instance), m_cmd_show(cmd_show) {
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = main_window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    RegisterClassW(&wc);

    CreateWindowExW(0,
                    MAIN_WINDOW_CLASS_NAME,
                    MAIN_WINDOW_TITLE,
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    nullptr,
                    nullptr,
                    instance,
                    static_cast<void *>(this));
}

std::wstring main_window::hwnd_window_text(HWND hwnd) noexcept {
    const auto hwnd_string_size = GetWindowTextLengthW(hwnd);
    std::wstring hwnd_string;
    hwnd_string.resize(static_cast<std::size_t>(hwnd_string_size));
    GetWindowTextW(hwnd, hwnd_string.data(), hwnd_string_size);
    return hwnd_string;
}

void main_window::hwnd_position_and_size(
    HWND hwnd, int x, int y, int width, int height) noexcept {
    SetWindowPos(
        hwnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT main_window::main_window_proc(HWND hwnd,
                                      UINT message_code,
                                      WPARAM w_param,
                                      LPARAM l_param) noexcept {
    const auto get_this = [hwnd]() -> main_window * {
        return reinterpret_cast<main_window *>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    };

    switch (message_code) {
    case WM_NCCREATE: {
        auto *create_struct = reinterpret_cast<CREATESTRUCT *>(l_param);
        auto *this_ =
            reinterpret_cast<main_window *>(create_struct->lpCreateParams);
        SetWindowLongPtrW(
            hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this_));
        this_->m_hwnd = hwnd;
        return DefWindowProcW(hwnd, message_code, w_param, l_param);
    }
    case WM_CREATE: {
        auto *this_ = get_this();
        if (this_ != nullptr) {
            this_->on_create();
        }
        return 0;
    }

    case WM_SIZE: {
        const auto width = LOWORD(l_param);
        const auto height = HIWORD(l_param);
        auto *this_ = get_this();
        if (this_ != nullptr) {
            this_->on_size(width, height);
        }
        return DefWindowProcW(hwnd, message_code, w_param, l_param);
    }

    case WM_GETDPISCALEDSIZE: {
        int new_dpi = static_cast<int>(w_param);
        SIZE *new_size = reinterpret_cast<SIZE *>(l_param);
        auto *this_ = get_this();
        if (this_ != nullptr) {
            this_->on_get_dpi_scaled_size(new_dpi, *new_size);
        }
        return TRUE;
    }

    case WM_DPICHANGED: {
        auto *new_rect = reinterpret_cast<RECT *>(l_param);
        auto *this_ = get_this();
        if (this_ != nullptr) {
            this_->on_dpi_changed(*new_rect);
        }
        return 0;
    }

    case WM_GETMINMAXINFO: {
        auto *min_max_info = reinterpret_cast<MINMAXINFO *>(l_param);
        LONG &minimum_width = min_max_info->ptMinTrackSize.x;
        LONG &minimum_height = min_max_info->ptMinTrackSize.y;
        auto *this_ = get_this();
        if (this_ != nullptr) {
            this_->on_get_min_max_info(minimum_width, minimum_height);
        }
        return 0;
    }

    case WM_SETTINGCHANGE: {
        if (is_color_scheme_change(l_param)) {
            auto *this_ = get_this();
            if (this_ != nullptr) {
                this_->on_color_scheme_changed();
            }
        }
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

void main_window::on_create() noexcept {
    m_current_dpi = static_cast<int>(GetDpiForWindow(m_hwnd));

    enable_dark_mode(m_hwnd, true);
    if (is_dark_mode_enabled()) {
        refresh_non_client_area(m_hwnd);
    }

    LOGFONT log_font;
    SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                               sizeof(log_font),
                               &log_font,
                               FALSE,
                               static_cast<UINT>(m_current_dpi));

    m_font = font(log_font);

    m_button_receive = CreateWindowExW(0,
                                       WC_BUTTON,
                                       L"Receive...",
                                       WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                       0,
                                       0,
                                       0,
                                       0,
                                       m_hwnd,
                                       nullptr,
                                       m_instance,
                                       nullptr);

    m_label_or = CreateWindowExW(0,
                                 WC_STATIC,
                                 L"Or",
                                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                                 0,
                                 0,
                                 0,
                                 0,
                                 m_hwnd,
                                 nullptr,
                                 m_instance,
                                 nullptr);

    m_separator_line_left = CreateWindowExW(0,
                                            WC_STATIC,
                                            nullptr,
                                            WS_VISIBLE | WS_CHILD,
                                            0,
                                            0,
                                            0,
                                            0,
                                            m_hwnd,
                                            nullptr,
                                            m_instance,
                                            nullptr);
    SetWindowSubclass(m_separator_line_left, line_proc, 0, 0);

    m_separator_line_right = CreateWindowExW(0,
                                             WC_STATIC,
                                             nullptr,
                                             WS_VISIBLE | WS_CHILD,
                                             0,
                                             0,
                                             0,
                                             0,
                                             m_hwnd,
                                             nullptr,
                                             m_instance,
                                             nullptr);
    SetWindowSubclass(m_separator_line_right, line_proc, 0, 0);

    m_label_to_device = CreateWindowExW(0,
                                        WC_STATIC,
                                        L"To:",
                                        WS_VISIBLE | WS_CHILD | SS_LEFT,
                                        0,
                                        0,
                                        0,
                                        0,
                                        m_hwnd,
                                        nullptr,
                                        m_instance,
                                        nullptr);

    m_text_field_to_device = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_EDIT,
        nullptr,
        WS_VISIBLE | WS_CHILD | ES_WANTRETURN | ES_MULTILINE | ES_AUTOHSCROLL,
        0,
        0,
        0,
        0,
        m_hwnd,
        nullptr,
        m_instance,
        nullptr);
    SetWindowSubclass(
        m_text_field_to_device, text_field_proc, ID_TEXT_FIELD_TO_DEVICE, 0);

    m_label_message_content = CreateWindowExW(0,
                                              WC_STATIC,
                                              L"Content:",
                                              WS_VISIBLE | WS_CHILD | SS_LEFT,
                                              0,
                                              0,
                                              0,
                                              0,
                                              m_hwnd,
                                              nullptr,
                                              m_instance,
                                              nullptr);

    m_text_field_message_content = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_EDIT,
        nullptr,
        WS_VISIBLE | WS_CHILD | ES_WANTRETURN | ES_MULTILINE | ES_AUTOVSCROLL,
        0,
        0,
        0,
        0,
        m_hwnd,
        nullptr,
        m_instance,
        nullptr);
    SetWindowSubclass(m_text_field_message_content,
                      text_field_proc,
                      ID_TEXT_FIELD_MESSAGE_CONTENT,
                      0);

    m_group_box_message_type =
        CreateWindowExW(0,
                        WC_BUTTON,
                        L"Message type",
                        WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                        0,
                        0,
                        0,
                        0,
                        m_hwnd,
                        nullptr,
                        m_instance,
                        nullptr);

    m_radio_button_message_type_url =
        CreateWindowExW(0,
                        WC_BUTTON,
                        L"URL",
                        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                        0,
                        0,
                        0,
                        0,
                        m_hwnd,
                        nullptr,
                        m_instance,
                        nullptr);

    Button_SetCheck(m_radio_button_message_type_url, BST_CHECKED);

    m_radio_button_message_type_text =
        CreateWindowExW(0,
                        WC_BUTTON,
                        L"Text",
                        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                        0,
                        0,
                        0,
                        0,
                        m_hwnd,
                        nullptr,
                        m_instance,
                        nullptr);

    m_button_send = CreateWindowExW(0,
                                    WC_BUTTON,
                                    L"Send...",
                                    WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                    0,
                                    0,
                                    0,
                                    0,
                                    m_hwnd,
                                    nullptr,
                                    m_instance,
                                    nullptr);

    apply_font();

    // 1. GetClientRect returns (0, 0) as origin and the client area width
    // and height.
    RECT main_window_client_rect;
    GetClientRect(m_hwnd, &main_window_client_rect);

    // 2. MapWindowPoints translates the origin to absolute screen
    // coordinates.
    MapWindowPoints(m_hwnd,
                    nullptr,
                    reinterpret_cast<POINT *>(&main_window_client_rect),
                    2);

    // 3. The desired width and height are added to the origin coordinates.
    main_window_client_rect.right =
        main_window_client_rect.left +
        MulDiv(WINDOW_WIDTH_96, m_current_dpi, USER_DEFAULT_SCREEN_DPI);
    main_window_client_rect.bottom =
        main_window_client_rect.top +
        MulDiv(WINDOW_HEIGHT_96, m_current_dpi, USER_DEFAULT_SCREEN_DPI);

    // 4. The absolute coordinates are adjusted for the non-client area.
    AdjustWindowRectExForDpi(&main_window_client_rect,
                             WS_OVERLAPPEDWINDOW,
                             FALSE,
                             0,
                             static_cast<UINT>(m_current_dpi));

    // 5. The absolute coordinates are used for the origin, and the
    // window's width and height is calculated via the right and bottom
    // coordinates.
    MoveWindow(m_hwnd,
               main_window_client_rect.left,
               main_window_client_rect.top,
               main_window_client_rect.right - main_window_client_rect.left,
               main_window_client_rect.bottom - main_window_client_rect.top,
               TRUE);

    ShowWindow(m_hwnd, m_cmd_show);
}

void main_window::on_size(int width, int height) noexcept {
    HDC hdc = GetDC(m_hwnd);

    const auto [width_label_or, height_label_or] =
        size_for_text(hdc, hwnd_window_text(m_label_or));

    hwnd_position_and_size(m_label_or,
                           ((width / 2) - (width_label_or / 2)),
                           ((height / 2) - (height_label_or / 2)),
                           width_label_or,
                           height_label_or);

    hwnd_position_and_size(
        m_separator_line_left,
        /* x: */ dpiscaled(LINE_PADDING),
        /* y: */ (height / 2),
        /* width: */
        ((width / 2) - (width_label_or / 2) - (dpiscaled(LINE_PADDING) * 2)),
        /* height: */ LINE_HEIGHT);

    hwnd_position_and_size(
        m_separator_line_right,
        /* x: */ (width / 2) + (width_label_or / 2) + dpiscaled(LINE_PADDING),
        /* y: */ (height / 2),
        /* width: */
        ((width / 2) - (width_label_or / 2) - (dpiscaled(LINE_PADDING) * 2)),
        /* height: */ LINE_HEIGHT);

    const auto [width_button_receive, height_button_receive] =
        size_for_button(m_button_receive, true);

    hwnd_position_and_size(
        m_button_receive,
        /* x: */ ((width / 2) - (width_button_receive / 2)),
        /* y: */ ((height / 4) - (height_button_receive / 2)),
        /* width: */ width_button_receive,
        /* height: */ height_button_receive);

    const auto [width_label_to_device, height_label_to_device] =
        size_for_text(hdc, hwnd_window_text(m_label_to_device));

    const auto bottom_half_y =
        (height / 2) + (height_label_or / 2) + dpiscaled(CONTENT_PADDING);

    hwnd_position_and_size(m_label_to_device,
                           /* x: */ dpiscaled(CONTENT_PADDING),
                           /* y: */ bottom_half_y,
                           /* width: */ width_label_to_device,
                           /* height: */ height_label_to_device);

    const auto text_field_to_device_y =
        bottom_half_y + height_label_to_device +
        dpiscaled(LABEL_TO_TEXT_FIELD_PADDING_96);

    hwnd_position_and_size(
        m_text_field_to_device,
        /* x: */ dpiscaled(CONTENT_PADDING),
        /* y: */ text_field_to_device_y,
        /* width: */ (width / 2) - (2 * dpiscaled(CONTENT_PADDING)),
        /* height: */ dpiscaled(TEXT_FIELD_SINGLE_LINE_HEIGHT_MINIMUM_96));

    const auto [width_label_message_content, height_label_message_content] =
        size_for_text(hdc, hwnd_window_text(m_label_message_content));

    const auto label_message_content_y =
        text_field_to_device_y +
        dpiscaled(TEXT_FIELD_SINGLE_LINE_HEIGHT_MINIMUM_96) +
        dpiscaled(CONTENT_PADDING);

    hwnd_position_and_size(m_label_message_content,
                           /* x: */ dpiscaled(CONTENT_PADDING),
                           /* y: */ label_message_content_y,
                           /* width: */ width_label_message_content,
                           /* height: */ height_label_message_content);

    const auto text_field_message_content_y =
        label_message_content_y + height_label_message_content +
        dpiscaled(LABEL_TO_TEXT_FIELD_PADDING_96);

    hwnd_position_and_size(
        m_text_field_message_content,
        /* x: */ dpiscaled(CONTENT_PADDING),
        /* y: */ text_field_message_content_y,
        /* width: */ (width / 2) - (2 * dpiscaled(CONTENT_PADDING)),
        /* height: */ height - text_field_message_content_y -
            dpiscaled(CONTENT_PADDING));

    const auto [width_radio_button_message_type_url,
                height_radio_button_message_type_url] =
        size_for_button(m_radio_button_message_type_url, false);

    const auto radio_button_message_type_url_y =
        bottom_half_y + dpiscaled(GROUP_BOX_TOP_PADDING_96);

    hwnd_position_and_size(m_radio_button_message_type_url,
                           /* x: */ (width / 2) + dpiscaled(CONTENT_PADDING) +
                               dpiscaled(GROUP_BOX_LEFT_PADDING_96),
                           /* y: */ radio_button_message_type_url_y,
                           /* width: */ width_radio_button_message_type_url,
                           /* height: */ height_radio_button_message_type_url);

    const auto [width_radio_button_message_type_text,
                height_radio_button_message_type_text] =
        size_for_button(m_radio_button_message_type_text, false);

    const auto radio_button_message_type_text_y =
        radio_button_message_type_url_y +
        height_radio_button_message_type_url +
        dpiscaled(GROUP_BOX_BETWEEN_CHILDREN_PADDING_96);

    hwnd_position_and_size(
        m_radio_button_message_type_text,
        /* x: */ (width / 2) + dpiscaled(CONTENT_PADDING) +
            dpiscaled(GROUP_BOX_LEFT_PADDING_96),
        /* y: */ radio_button_message_type_text_y,
        /* width: */ width_radio_button_message_type_text,
        /* height: */ height_radio_button_message_type_text);

    hwnd_position_and_size(
        m_group_box_message_type,
        /* x: */ (width / 2) + dpiscaled(CONTENT_PADDING),
        /* y: */ bottom_half_y,
        /* width: */ (width / 2) - (2 * dpiscaled(CONTENT_PADDING)),
        /* height: */ dpiscaled(GROUP_BOX_TOP_PADDING_96) +
            height_radio_button_message_type_url +
            dpiscaled(GROUP_BOX_BETWEEN_CHILDREN_PADDING_96) +
            height_radio_button_message_type_text +
            dpiscaled(GROUP_BOX_BOTTOM_PADDING_96));

    const auto [width_button_send, height_button_send] =
        size_for_button(m_button_send, true);

    hwnd_position_and_size(
        m_button_send,
        /* x: */
        (width + 1 - width_button_send - dpiscaled(CONTENT_PADDING)),
        /* y: */
        (height + 1 - height_button_send - dpiscaled(CONTENT_PADDING)),
        /* width: */ width_button_send,
        /* height: */ height_button_send);

    ReleaseDC(m_hwnd, hdc);
}

void main_window::on_get_dpi_scaled_size(int new_dpi,
                                         SIZE &new_size) noexcept {
    const auto scaling_factor = static_cast<double>(new_dpi) / m_current_dpi;

    RECT client_area;
    GetClientRect(m_hwnd, &client_area);

    client_area.right = static_cast<LONG>(
        static_cast<double>(client_area.right) * scaling_factor);
    client_area.bottom = static_cast<LONG>(
        static_cast<double>(client_area.bottom) * scaling_factor);

    AdjustWindowRectExForDpi(&client_area,
                             WS_OVERLAPPEDWINDOW,
                             FALSE,
                             0,
                             static_cast<UINT>(new_dpi));

    new_size.cx = client_area.right - client_area.left;
    new_size.cy = client_area.bottom - client_area.top;

    m_current_dpi = new_dpi;
}

void main_window::on_dpi_changed(RECT &new_rect) noexcept {
    LOGFONT log_font;
    SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT,
                               sizeof(log_font),
                               &log_font,
                               FALSE,
                               static_cast<UINT>(m_current_dpi));

    m_font = font(log_font);
    apply_font();

    hwnd_position_and_size(m_hwnd,
                           new_rect.left,
                           new_rect.top,
                           new_rect.right - new_rect.left,
                           new_rect.bottom - new_rect.top);
}

void main_window::on_get_min_max_info(LONG &minimum_width,
                                      LONG &minimum_height) noexcept {
    HDC hdc = GetDC(m_hwnd);

    const auto height_label_or = [hdc, this]() -> LONG {
        const auto size_label_or =
            size_for_text(hdc, hwnd_window_text(m_label_or));
        return size_label_or.cy;
    }();

    const auto button_receive_size = size_for_button(m_button_receive, true);

    const auto [width_label_to_device, height_label_to_device] =
        size_for_text(hdc, hwnd_window_text(m_label_to_device));

    const auto [width_label_message_content, height_label_message_content] =
        size_for_text(hdc, hwnd_window_text(m_label_message_content));

    const auto radio_button_message_type_text_size =
        size_for_button(m_radio_button_message_type_text, false);

    const auto radio_button_message_type_url_size =
        size_for_button(m_radio_button_message_type_url, false);

    const auto width_group_box_message_type = [hdc, this]() -> LONG {
        const auto size_group_box_message_type =
            size_for_text(hdc, hwnd_window_text(m_group_box_message_type));
        return size_group_box_message_type.cx;
    }();

    const auto button_send_size = size_for_button(m_button_send, true);

    // Height

    const auto minimum_height_lower_left =
        dpiscaled(CONTENT_PADDING) + height_label_to_device +
        dpiscaled(LABEL_TO_TEXT_FIELD_PADDING_96) +
        dpiscaled(TEXT_FIELD_SINGLE_LINE_HEIGHT_MINIMUM_96) + dpiscaled(2) +
        dpiscaled(CONTENT_PADDING) + height_label_message_content +
        dpiscaled(LABEL_TO_TEXT_FIELD_PADDING_96) +
        dpiscaled(TEXT_FIELD_SINGLE_LINE_HEIGHT_MINIMUM_96) + dpiscaled(2) +
        dpiscaled(CONTENT_PADDING);

    const auto minimum_height_lower_right =
        dpiscaled(CONTENT_PADDING) + dpiscaled(GROUP_BOX_TOP_PADDING_96) +
        radio_button_message_type_url_size.cy +
        dpiscaled(GROUP_BOX_BETWEEN_CHILDREN_PADDING_96) +
        radio_button_message_type_text_size.cy +
        dpiscaled(GROUP_BOX_BOTTOM_PADDING_96) + dpiscaled(CONTENT_PADDING) +
        button_send_size.cy + dpiscaled(CONTENT_PADDING);

    const std::array<int, 2> height_lower = {minimum_height_lower_left,
                                             minimum_height_lower_right};
    const auto minimum_height_lower =
        *std::max_element(std::begin(height_lower), std::end(height_lower));

    minimum_height = height_label_or +
                     (2 * GetSystemMetricsForDpi(
                              SM_CYFRAME, static_cast<UINT>(m_current_dpi))) +
                     GetSystemMetricsForDpi(SM_CYCAPTION,
                                            static_cast<UINT>(m_current_dpi)) +
                     (2 * minimum_height_lower);

    // Width

    const auto minimum_width_upper = button_receive_size.cx;

    const std::array<int, 3> widths_lower_left = {
        width_label_to_device,
        dpiscaled(TEXT_FIELD_WIDTH_MINIMUM_96) + dpiscaled(4),
        width_label_message_content};

    const auto minimum_width_lower_left = *std::max_element(
        std::begin(widths_lower_left), std::end(widths_lower_left));

    const std::array<int, 4> widths_lower_right = {
        width_group_box_message_type,
        (dpiscaled(GROUP_BOX_LEFT_PADDING_96) +
         radio_button_message_type_url_size.cx) +
            dpiscaled(GROUP_BOX_LEFT_PADDING_96),
        (dpiscaled(GROUP_BOX_LEFT_PADDING_96) +
         radio_button_message_type_text_size.cx) +
            dpiscaled(GROUP_BOX_LEFT_PADDING_96),
        button_send_size.cx};

    const auto minimum_width_lower_right = *std::max_element(
        std::begin(widths_lower_right), std::end(widths_lower_right));

    const std::array<int, 2> widths_lower = {minimum_width_lower_left,
                                             minimum_width_lower_right};
    const auto minimum_width_lower =
        *std::max_element(std::begin(widths_lower), std::end(widths_lower));

    const std::array<int, 2> widths = {minimum_width_upper,
                                       minimum_width_lower};

    minimum_width =
        (2 * GetSystemMetricsForDpi(SM_CXFRAME,
                                    static_cast<UINT>(m_current_dpi))) +
        (4 * dpiscaled(CONTENT_PADDING)) +
        (2 * *std::max_element(std::begin(widths), std::end(widths)));

    ReleaseDC(m_hwnd, hdc);
}

void main_window::on_color_scheme_changed() noexcept {
    refresh_non_client_area(m_hwnd);
}

void main_window::apply_font() noexcept {
    auto *system_font = m_font.get();
    const auto apply_to = [system_font](HWND hwnd) -> void {
        SendMessageW(hwnd,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));
    };

    apply_to(m_button_receive);
    apply_to(m_label_or);
    apply_to(m_label_to_device);
    apply_to(m_text_field_to_device);
    apply_to(m_label_message_content);
    apply_to(m_text_field_message_content);
    apply_to(m_group_box_message_type);
    apply_to(m_radio_button_message_type_url);
    apply_to(m_radio_button_message_type_text);
    apply_to(m_button_send);
}

int main_window::dpiscaled(int value) const noexcept {
    return MulDiv(value, m_current_dpi, USER_DEFAULT_SCREEN_DPI);
}

SIZE main_window::size_for_text(HDC hdc, const std::wstring &text) noexcept {
    SIZE text_size;
    GetTextExtentPoint32W(
        hdc, text.data(), static_cast<int>(text.size()), &text_size);
    text_size.cx = dpiscaled(text_size.cx);
    text_size.cy = dpiscaled(text_size.cy);
    return text_size;
}

SIZE main_window::size_for_button(HWND button,
                                  bool extend_width_if_needed) noexcept {
    const int button_width_minimum = dpiscaled(BUTTON_WIDTH_MINIMUM_96);
    SIZE button_size;
    Button_GetIdealSize(button, &button_size);

    if (extend_width_if_needed && button_size.cx < button_width_minimum) {
        button_size.cx = button_width_minimum;
    }

    return button_size;
}

} // namespace linkollector::win
