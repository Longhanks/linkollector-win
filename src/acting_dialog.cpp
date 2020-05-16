#include "acting_dialog.h"

#include <algorithm>
#include <vector>

namespace linkollector::win {

static INT_PTR CALLBACK dialog_proc(HWND hwnd,
                                    UINT message_code,
                                    WPARAM w_param,
                                    LPARAM l_param) {
    switch (message_code) {
    case WM_INITDIALOG: {
        SetWindowPos(hwnd,
                     nullptr,
                     0,
                     0,
                     312,
                     140,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        return TRUE;
    }

    case WM_COMMAND: {
        switch (LOWORD(w_param)) {
        case IDOK:
            EndDialog(hwnd, TRUE);
            return TRUE;
        }
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

    return DialogBoxIndirectParamW(instance,
                                   reinterpret_cast<LPDLGTEMPLATE>(buf.data()),
                                   parent,
                                   dialog_proc,
                                   0L);
}

} // namespace linkollector::win
