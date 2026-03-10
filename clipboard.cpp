#include "clipboard.h"

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

// ===================== Keyboard Layouts =====================
std::string GetLayoutName(HKL hkl) {
    LANGID langId = LOWORD(hkl);
    LCID   lcid   = MAKELCID(langId, SORT_DEFAULT);
    char langName[256] = {};
    GetLocaleInfoA(lcid, LOCALE_SLANGUAGE, langName, sizeof(langName));
    if (langName[0] == '\0')
        snprintf(langName, sizeof(langName), "Unknown (LANGID=0x%04X)", langId);
    return std::string(langName);
}

void ShowInstalledLayouts() {
    int count = GetKeyboardLayoutList(0, nullptr);
    if (count == 0) { std::cout << "[!] Could get the list of layouts.\n"; return; }

    std::vector<HKL> layouts(count);
    GetKeyboardLayoutList(count, layouts.data());

    std::cout << "=== Installed Layouts (" << count << ") ===\n";
    for (int i = 0; i < count; ++i) {
        std::cout << "  [" << i << "]  HKL=0x" << std::hex << (UINT_PTR)layouts[i]
                  << std::dec << "  ->  " << GetLayoutName(layouts[i]) << "\n";
    }
    std::cout << "\n";
}

void ShowCurrentLayout() {
    HWND  hwnd     = GetForegroundWindow();
    DWORD threadId = GetWindowThreadProcessId(hwnd, nullptr);
    HKL   hkl      = GetKeyboardLayout(threadId);
    std::cout << "=== Текущая раскладка ===\n";
    std::cout << "  HKL  = 0x" << std::hex << (UINT_PTR)hkl << std::dec << "\n";
    std::cout << "  Язык = " << GetLayoutName(hkl) << "\n\n";
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
