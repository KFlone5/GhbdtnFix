#include <windows.h>
#include <iostream>
#include <locale.h>
#include "clipboard.h"
#include "textutils.h"

// ===================== MAIN =====================
int main() {
    setlocale(LC_ALL, "");

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

    std::cout << "GhbdtnFix running...\n";
    std::cout << "Ctrl + 1  -> Type Clipboard\n";
    std::cout << "Ctrl + 9  -> Change Keyboard Layout\n";
    std::cout << "Ctrl + 0  -> Remove_Spaces\n";
    std::cout << "Ctrl + ;  -> Invert Case\n";
    std::cout << "Ctrl + [  -> lowercase\n";
    std::cout << "Ctrl + ]  -> UPPERCASE\n";
    std::cout << "\n";

    ShowInstalledLayouts();
    ShowCurrentLayout();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {

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
                TypeText(text);

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

    return 0;
}
