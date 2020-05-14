#include "dark_mode.h"
#include "main_window.h"

#include <cstdlib>

#include <Windows.h>

#include <CommCtrl.h>

int WINAPI wWinMain(HINSTANCE instance,
                    [[maybe_unused]] HINSTANCE hPrevInstance,
                    [[maybe_unused]] PWSTR pCmdLine,
                    int cmd_show) {
    linkollector::win::init_dark_mode_support();

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

    linkollector::win::main_window main_window(instance, cmd_show);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) != 0) {
        if (main_window.get() != nullptr &&
            IsDialogMessageW(main_window.get(), &msg) == TRUE) {
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return EXIT_SUCCESS;
}
