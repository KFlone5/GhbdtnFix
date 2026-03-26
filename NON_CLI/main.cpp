#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#include <windows.h>
#include "clipboard.h"
#include "textutils.h"

// ===================== MAIN =====================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // Ctrl + 9 - Change Keyboard Layout
    RegisterHotKey(nullptr, 1, MOD_CONTROL, '9');

    // Ctrl + ;  - Invert Case
    RegisterHotKey(nullptr, 2, MOD_CONTROL, VK_OEM_1);

    // Ctrl + [  - lowercase
    RegisterHotKey(nullptr, 3, MOD_CONTROL, VK_OEM_4);

    // Ctrl + ]  - UPPERCASE
    RegisterHotKey(nullptr, 4, MOD_CONTROL, VK_OEM_6);

    // Ctrl + 0  - RemoveSpaces
    RegisterHotKey(nullptr, 5, MOD_CONTROL, '0');

    // Ctrl + 1  - Type Clipboard
    RegisterHotKey(nullptr, 6, MOD_CONTROL, '1');

    // Ctrl + Alt + Q  - Exit
    RegisterHotKey(nullptr, 7, MOD_CONTROL | MOD_ALT, 'Q');

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {

            // Exit
            if (msg.wParam == 7)
                break;

            // Save clipboard before we overwrite it with Ctrl+C
            std::wstring previousClipboard = GetClipboardTextW();

            Sleep(50);
            SendCtrlC();
            Sleep(100);

            std::wstring text = GetClipboardTextW();
            if (text.empty())
                continue;

            if (msg.wParam == 1) {
                TypeText(RemapLayoutText(text));
                SwitchToNextLayout();
            }
            else if (msg.wParam == 2)
                TypeText(InvertCase(text));
            else if (msg.wParam == 3)
                TypeText(LowerCase(text));
            else if (msg.wParam == 4)
                TypeText(UpperCase(text));
            else if (msg.wParam == 5)
                TypeText(RemoveSpaces(text));
            else if (msg.wParam == 6)
                TypeText(RemoveSpaces(text));

            // Restore clipboard to what it was before
            SetClipboardTextW(previousClipboard);
        }
    }

    UnregisterHotKey(nullptr, 1);
    UnregisterHotKey(nullptr, 2);
    UnregisterHotKey(nullptr, 3);
    UnregisterHotKey(nullptr, 4);
    UnregisterHotKey(nullptr, 5);
    UnregisterHotKey(nullptr, 6);
    UnregisterHotKey(nullptr, 7);

    return 0;
}
