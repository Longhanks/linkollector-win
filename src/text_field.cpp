#include "text_field.h"

#include "constants.h"

#include <cmath>
#include <string>

#include <CommCtrl.h>

namespace linkollector::win {

LRESULT CALLBACK
text_field_proc(HWND hwnd,
                UINT message_code,
                WPARAM w_param,
                LPARAM l_param,
                UINT_PTR subclass_id,
                [[maybe_unused]] DWORD_PTR ref_data) noexcept {
    if (message_code != WM_PAINT && message_code != WM_GETDLGCODE) {
        return DefSubclassProc(hwnd, message_code, w_param, l_param);
    }

    if (message_code == WM_GETDLGCODE) {
        auto return_value =
            DefSubclassProc(hwnd, message_code, w_param, l_param);
        return_value &= ~DLGC_WANTALLKEYS;
        return return_value;
    }

    const auto text_field_string_size = GetWindowTextLengthW(hwnd);

    if (text_field_string_size > 0) {
        return DefSubclassProc(hwnd, message_code, w_param, l_param);
    }

    RECT rect;
    GetClientRect(hwnd, &rect);

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    HBRUSH brush = CreateSolidBrush(GetBkColor(hdc));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    std::wstring txt;

    if (subclass_id == ID_TEXT_FIELD_TO_DEVICE) {
        txt = L"Device";
    }

    else if (subclass_id == ID_TEXT_FIELD_MESSAGE_CONTENT) {
        txt = L"URL/Text...";
    }

    auto *font = reinterpret_cast<HFONT>(SendMessageW(hwnd, WM_GETFONT, 0, 0));

    HGDIOBJ original_font = SelectObject(hdc, font);

    COLORREF original_text_color =
        SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));

    TEXTMETRICW text_metrics;
    GetTextMetricsW(hdc, &text_metrics);

    const double overhang_padding = text_metrics.tmHeight / 6.0;
    const auto left_margin = static_cast<int>(std::ceil(overhang_padding));
    const auto right_margin =
        static_cast<int>(std::ceil(overhang_padding + (1.5)));

    DRAWTEXTPARAMS draw_text_params;
    draw_text_params.cbSize = sizeof(DRAWTEXTPARAMS);
    draw_text_params.iTabLength = 0;
    draw_text_params.iLeftMargin = left_margin;
    draw_text_params.iRightMargin = right_margin;

    // The WS_EX_CLIENTEDGE
    rect.left += 1;
    rect.top += 1;

    DrawTextExW(hdc,
                txt.data(),
                static_cast<int>(txt.size()),
                &rect,
                DT_TOP | DT_LEFT | DT_END_ELLIPSIS,
                &draw_text_params);

    SetTextColor(hdc, original_text_color);
    SelectObject(hdc, original_font);

    EndPaint(hwnd, &ps);
    return DefSubclassProc(hwnd, message_code, w_param, l_param);
}

} // namespace linkollector::win
