#pragma once

#include "font.h"

#include <Windows.h>

#include <string>

namespace linkollector::win {

class main_window final {

public:
    explicit main_window(HINSTANCE instance, int cmd_show) noexcept;
    main_window(const main_window &other) = delete;
    main_window &operator=(const main_window &other) = delete;
    main_window(main_window &&other) noexcept = default;
    main_window &operator=(main_window &&other) noexcept = default;
    ~main_window() noexcept = default;

    [[nodiscard]] HWND get() noexcept;

private:
    static inline constexpr const wchar_t *MAIN_WINDOW_CLASS_NAME =
        L"Linkollector Main Window";
    static inline constexpr const wchar_t *MAIN_WINDOW_TITLE = L"Linkollector";

    [[nodiscard]] static std::wstring hwnd_window_text(HWND hwnd) noexcept;
    static void hwnd_position_and_size(
        HWND hwnd, int x, int y, int width, int height) noexcept;

    static LRESULT CALLBACK main_window_proc(HWND hwnd,
                                             UINT message_code,
                                             WPARAM w_param,
                                             LPARAM l_param) noexcept;

    void on_create() noexcept;
    void on_size(int width, int height) noexcept;
    void on_activate(UINT state) noexcept;
    void on_set_focus() noexcept;
    void on_get_dpi_scaled_size(int new_dpi, SIZE &new_size) noexcept;
    void on_dpi_changed(RECT &new_rect) noexcept;
    void on_get_min_max_info(LONG &minimum_width,
                             LONG &minimum_height) noexcept;
    void on_color_scheme_changed() noexcept;
    void on_text_changed(HWND text_field) noexcept;

    void apply_font() noexcept;
    [[nodiscard]] int dpiscaled(int value) const noexcept;
    [[nodiscard]] SIZE size_for_text(HDC hdc,
                                     const std::wstring &text) noexcept;
    [[nodiscard]] SIZE size_for_button(HWND button,
                                       bool extend_width_if_needed) noexcept;

    HINSTANCE m_instance = nullptr;
    HWND m_hwnd = nullptr;
    int m_cmd_show = -1;
    int m_current_dpi = -1;
    font m_font;

    HWND m_hwnd_last_focus = nullptr;

    HWND m_button_receive = nullptr;
    HWND m_separator_line_left = nullptr;
    HWND m_label_or = nullptr;
    HWND m_separator_line_right = nullptr;
    HWND m_label_to_device = nullptr;
    HWND m_text_field_to_device = nullptr;
    HWND m_label_message_content = nullptr;
    HWND m_text_field_message_content = nullptr;
    HWND m_group_box_message_type = nullptr;
    HWND m_radio_button_message_type_url = nullptr;
    HWND m_radio_button_message_type_text = nullptr;
    HWND m_button_send = nullptr;
};

} // namespace linkollector::win
