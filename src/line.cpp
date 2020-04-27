#include "line.h"

#include <CommCtrl.h>

LRESULT CALLBACK line_window_f(HWND hwnd,
                               UINT message_code,
                               WPARAM w_param,
                               LPARAM l_param,
                               [[maybe_unused]] UINT_PTR subclass_id,
                               [[maybe_unused]] DWORD_PTR ref_data) noexcept {
    if (message_code != WM_PAINT) {
        return DefSubclassProc(hwnd, message_code, w_param, l_param);
    }

    RECT rect;
    GetClientRect(hwnd, &rect);

    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);
    HGDIOBJ original = SelectObject(hdc, GetStockObject(DC_PEN));
    SelectObject(hdc, GetStockObject(DC_PEN));

    SetDCPenColor(hdc, RGB(160, 160, 160));
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

    SelectObject(hdc, original);
    EndPaint(hwnd, &ps);
    return 0;
}
