#include <windows.h>
#include <iostream>
#include <locale.h>
#include "clipboard.h"
#include "textutils.h"

// ===================== MAIN =====================
int main() {
    setlocale(LC_ALL, "");

    // Ctrl + ;
    RegisterHotKey(nullptr, 1, MOD_CONTROL, VK_OEM_1);

    // Ctrl + [
    RegisterHotKey(nullptr, 2, MOD_CONTROL, VK_OEM_4);

    // Ctrl + ]
    RegisterHotKey(nullptr, 3, MOD_CONTROL, VK_OEM_6);

    // Ctrl + 9
    RegisterHotKey(nullptr, 4, MOD_CONTROL, '9');

    // Ctrl + 0
    RegisterHotKey(nullptr, 5, MOD_CONTROL, '0');

    std::cout << "Running...\n";
    std::cout << "Ctrl + ;  -> Invert Case\n";
    std::cout << "Ctrl + [  -> lower case\n";
    std::cout << "Ctrl + ]  -> UPPER CASE\n";
    std::cout << "Ctrl + 9  -> Change Keyboard Layout\n";
    std::cout << "Ctrl + 0  -> Remove Spaces\n";

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {

            Sleep(50);
            SendCtrlC();
            Sleep(100);

            std::wstring text = GetClipboardTextW();
            if (text.empty())
                continue;

            if (msg.wParam == 1)
                TypeText(InvertCase(text));
            else if (msg.wParam == 2)
                TypeText(LowerCase(text));
            else if (msg.wParam == 3)
                TypeText(UpperCase(text));
            // else if (msg.wParam == 4) {
            //     TypeText();
            else if (msg.wParam == 5) {
                TypeText(RemoveSpaces(text));
            }
        }
    }

    UnregisterHotKey(nullptr, 1);
    UnregisterHotKey(nullptr, 2);
    UnregisterHotKey(nullptr, 3);
    UnregisterHotKey(nullptr, 4);
    UnregisterHotKey(nullptr, 5);

    return 0;
}
