#pragma once

#include <Windows.h>

LRESULT CALLBACK text_field_window_f(HWND hwnd,
                                     UINT message_code,
                                     WPARAM w_param,
                                     LPARAM l_param,
                                     UINT_PTR subclass_id,
                                     DWORD_PTR ref_data) noexcept;
