#include <windows.h>
#include <string>
#include <iostream>
#include <cwctype>
#include <locale.h>

// ===================== COPY SELECTION =====================
void SendCtrlC() {
    INPUT inputs[4] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

// ===================== CLIPBOARD (UNICODE) =====================
std::wstring GetClipboardTextW() {
    if (!OpenClipboard(nullptr))
        return L"";

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        CloseClipboard();
        return L"";
    }

    wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
    if (!pszText) {
        CloseClipboard();
        return L"";
    }

    std::wstring text = pszText;

    GlobalUnlock(hData);
    CloseClipboard();
    return text;
}

// ===================== TEXT FUNCTIONS =====================
std::wstring InvertCase(const std::wstring& text) {
    std::wstring result = text;
    for (wchar_t& c : result) {
        if (iswlower(c))
            c = towupper(c);
        else if (iswupper(c))
            c = towlower(c);
    }
    return result;
}

std::wstring LowerCase(const std::wstring& text) {
    std::wstring result = text;
    for (wchar_t& c : result)
        c = towlower(c);
    return result;
}

std::wstring UpperCase(const std::wstring& text) {
    std::wstring result = text;
    for (wchar_t& c : result)
        c = towupper(c);
    return result;
}

std::wstring RemoveSpaces(const std::wstring& text) {
    std::wstring result;
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] != L' ') {
            result += text[i];
        }
    }
    return result;
}

// ===================== TYPE TEXT =====================
void TypeText(const std::wstring& text) {
    for (wchar_t c : text) {
        INPUT input[2] = {};

        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wScan = c;
        input[0].ki.dwFlags = KEYEVENTF_UNICODE;

        input[1] = input[0];
        input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

        SendInput(2, input, sizeof(INPUT));
    }
}

// ===================== MAIN =====================
int main() {
    setlocale(LC_ALL, "");

    // Ctrl + ;
    RegisterHotKey(nullptr, 1, MOD_CONTROL, VK_OEM_1);

    // Ctrl + [
    RegisterHotKey(nullptr, 2, MOD_CONTROL, VK_OEM_4);

    // Ctrl + ]
    RegisterHotKey(nullptr, 3, MOD_CONTROL, VK_OEM_6);

    // Ctrl + 0
    RegisterHotKey(nullptr, 4, MOD_CONTROL, '0');

    std::cout << "Running...\n";
    std::cout << "Ctrl + ;  -> invert case\n";
    std::cout << "Ctrl + [  -> lower case\n";
    std::cout << "Ctrl + ]  -> upper case\n";
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
            else if (msg.wParam == 4) {
                TypeText(RemoveSpaces(text));
            }
        }
    }

    UnregisterHotKey(nullptr, 1);
    UnregisterHotKey(nullptr, 2);
    UnregisterHotKey(nullptr, 3);
    UnregisterHotKey(nullptr, 4);

    return 0;
}
