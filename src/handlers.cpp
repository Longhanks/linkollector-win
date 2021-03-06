#include "handlers.h"

#include "constants.h"

#include <Windows.h>
#include <shellapi.h>

namespace linkollector::win {

void handle_url(const std::wstring &url) noexcept {
    ShellExecuteW(
        nullptr, nullptr, url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void handle_text(const std::wstring &text) noexcept {
    HWND notepad_window = nullptr;
    STARTUPINFOW startupInfo{};
    PROCESS_INFORMATION processInfo{};

    std::wstring notepad_exe(L"notepad.exe");

    if (CreateProcessW(nullptr,
                       notepad_exe.data(),
                       nullptr,
                       nullptr,
                       FALSE,
                       0,
                       nullptr,
                       nullptr,
                       &startupInfo,
                       &processInfo) > 0) {
        WaitForInputIdle(processInfo.hProcess, INFINITE);

        DWORD process_id = 0;
        HWND window = GetWindow(GetDesktopWindow(), GW_CHILD);
        while (window != nullptr) {
            DWORD thread_id = GetWindowThreadProcessId(window, &process_id);
            if ((thread_id == processInfo.dwThreadId) &&
                (process_id == processInfo.dwProcessId)) {
                std::wstring class_name;
                class_name.resize(WINDOW_CLASS_NAME_MAX_SIZE);

                const auto actual_size = static_cast<std::size_t>(
                    GetClassNameW(window,
                                  class_name.data(),
                                  WINDOW_CLASS_NAME_MAX_SIZE));

                class_name.resize(actual_size);

                if (class_name == L"Notepad") {
                    notepad_window = window;
                    break;
                }
            }
            window = GetWindow(window, GW_HWNDNEXT);
        }
    }

    auto *child = FindWindowExW(notepad_window, nullptr, L"Edit", nullptr);
    SendMessageW(child, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
}

} // namespace linkollector::win
