#include <cstdlib>
#include <string>

#include <Windows.h>
#include <shellapi.h>

#include <CommCtrl.h>

static void handle_url(const std::string &url) noexcept {
    const int wide_char_characters =
        MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, nullptr, 0);

    std::wstring out(static_cast<std::size_t>(wide_char_characters), L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, url.c_str(), -1, out.data(), wide_char_characters);

    ShellExecuteW(
        nullptr, nullptr, out.data(), nullptr, nullptr, SW_SHOWNORMAL);
}

static constexpr const int window_name_max_size = 256;

static void handle_text(const std::string &text) noexcept {
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
                class_name.resize(window_name_max_size);

                const auto actual_size =
                    static_cast<std::size_t>(GetClassNameW(
                        window, class_name.data(), window_name_max_size));

                class_name.resize(actual_size);

                if (class_name == L"Notepad") {
                    notepad_window = window;
                    break;
                }
            }
            window = GetWindow(window, GW_HWNDNEXT);
        }
    }

    const int wide_char_characters =
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);

    std::wstring out(static_cast<std::size_t>(wide_char_characters), L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, text.c_str(), -1, out.data(), wide_char_characters);

    auto *child = FindWindowExW(notepad_window, nullptr, L"Edit", nullptr);
    SendMessageW(child, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(out.data()));
}

static constexpr const wchar_t *MAIN_WINDOW_CLASS_NAME =
    L"Linkollector Main Main Window";

static HFONT system_font;
static HWND button_receive;
static HWND button_send;

static LRESULT CALLBACK main_window_proc(HWND hwnd,
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
            CreateWindowExW(0L,
                            L"BUTTON",
                            L"Receive",
                            WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                            55,
                            6,
                            75,
                            23,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);

        button_send =
            CreateWindowExW(0L,
                            L"BUTTON",
                            L"Send",
                            WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                            55,
                            33,
                            75,
                            23,
                            hwnd,
                            nullptr,
                            reinterpret_cast<HINSTANCE>(
                                GetWindowLongPtrW(hwnd, GWLP_HINSTANCE)),
                            nullptr);
        SendMessageW(button_receive,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

        SendMessageW(button_send,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(system_font),
                     MAKELPARAM(TRUE, 0));

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
    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = main_window_proc;
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
