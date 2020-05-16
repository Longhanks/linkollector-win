#pragma once

#include <Windows.h>

#include <string>

namespace linkollector::win {

LRESULT
show_acting_dialog(HINSTANCE instance, HWND parent, std::wstring_view title);

} // namespace linkollector::win
