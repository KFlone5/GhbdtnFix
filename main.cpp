#include <windows.h>
#include <string>
#include <iostream>

void SendCtrlC() {
    INPUT inputs[4] = {};

    // Ctrl down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // C down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';

    // C up
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Ctrl up
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

std::string GetClipboardText() {
    if (!OpenClipboard(nullptr))
        return "";

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        CloseClipboard();
        return "";
    }

    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        CloseClipboard();
        return "";
    }

    std::string text = pszText;
    GlobalUnlock(hData);
    CloseClipboard();
    return text;
}

// Toggle case (from 'a' to 'A' OR from 'A' to 'a')
std::string InvertCase(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        if (c >= 'a' && c <= 'z')
            c = c - 32;
        else if (c >= 'A' && c <= 'Z')
            c = c + 32;
    }
    return result;
}

// Make all text smaller
std::string LowerCase(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        if (c >= 'A' && c <= 'Z')
            c = c + 32;   // A -> a
    }
    return result;
}

// Make all text Uppercase
std::string UpperCase(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        if (c >= 'a' && c <= 'z')
            c = c - 32;   // a -> A
    }
    return result;
}

void TypeText(const std::string& text) {
    for (char c : text) {
        INPUT input[2] = {};

        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wScan = c;
        input[0].ki.dwFlags = KEYEVENTF_UNICODE;

        input[1] = input[0];
        input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

        SendInput(2, input, sizeof(INPUT));
    }
}

int main() {
    // Ctrl + ;
    RegisterHotKey(nullptr, 1, MOD_CONTROL, VK_OEM_1);

    // Ctrl + [
    RegisterHotKey(nullptr, 2, MOD_CONTROL, VK_OEM_4);

    // Ctrl + ]
    RegisterHotKey(nullptr, 3, MOD_CONTROL, VK_OEM_6);

    std::cout << "Running...\n";
    std::cout << "Ctrl + ;  -> invert case\n";
    std::cout << "Ctrl + [  -> lower case\n";
    std::cout << "Ctrl + ]  -> upper case\n";

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            Sleep(50);
            SendCtrlC();
            Sleep(100);

            std::string text = GetClipboardText();
            if (text.empty())
                continue;

            if (msg.wParam == 1) {
                TypeText(InvertCase(text));
            }
            else if (msg.wParam == 2) {
                TypeText(LowerCase(text));
            }
            else if (msg.wParam == 3) {
                TypeText(UpperCase(text));
            }
        }
    }

    UnregisterHotKey(nullptr, 1);
    UnregisterHotKey(nullptr, 2);
    UnregisterHotKey(nullptr, 3);

    return 0;
}
