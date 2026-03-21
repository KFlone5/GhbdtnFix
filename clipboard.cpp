#include "clipboard.h"
#include <iostream>

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

// ===================== GET LAYOUT NAME =====================
std::string GetLayoutName(HKL hkl) {
    LANGID langId = LOWORD(hkl);
    LCID   lcid   = MAKELCID(langId, SORT_DEFAULT);
    char langName[256] = {};
    GetLocaleInfoA(lcid, LOCALE_SLANGUAGE, langName, sizeof(langName));
    if (langName[0] == '\0')
        snprintf(langName, sizeof(langName), "Unknown (LANGID=0x%04X)", langId);
    return std::string(langName);
}

// ===================== SHOW ALL INSTALLED LAYOUTS =====================
void ShowInstalledLayouts() {
    int count = GetKeyboardLayoutList(0, nullptr);
    if (count == 0) {
        std::cout << "[!] Failed to retrieve layout list.\n";
        return;
    }

    std::vector<HKL> layouts(count);
    GetKeyboardLayoutList(count, layouts.data());

    std::cout << "=== Installed layouts (" << count << " total) ===\n";
    for (int i = 0; i < count; ++i) {
        std::cout << "  [" << i << "]  HKL=0x" << std::hex << (UINT_PTR)layouts[i]
                  << std::dec << "  ->  " << GetLayoutName(layouts[i]) << "\n";
    }
    std::cout << "\n";
}

// ===================== SHOW CURRENT LAYOUT =====================
void ShowCurrentLayout() {
    HWND  hwnd     = GetForegroundWindow();
    DWORD threadId = GetWindowThreadProcessId(hwnd, nullptr);
    HKL   hkl      = GetKeyboardLayout(threadId);

    std::cout << "=== Current layout ===\n";
    std::cout << "  HKL      = 0x" << std::hex << (UINT_PTR)hkl << std::dec << "\n";
    std::cout << "  Language = " << GetLayoutName(hkl) << "\n\n";
}

// ===================== SWITCH TO NEXT LAYOUT =====================
void SwitchToNextLayout() {
    int count = GetKeyboardLayoutList(0, nullptr);
    if (count < 2) {
        std::cout << "[!] Only one layout installed, nothing to switch to.\n";
        return;
    }

    std::vector<HKL> layouts(count);
    GetKeyboardLayoutList(count, layouts.data());

    HWND  hwnd         = GetForegroundWindow();
    DWORD targetThread = GetWindowThreadProcessId(hwnd, nullptr);
    DWORD ownThread    = GetCurrentThreadId();

    HKL current = GetKeyboardLayout(targetThread);
    std::string from = GetLayoutName(current);

    // Find the next layout in the list
    int idx = 0;
    for (int i = 0; i < count; ++i)
        if (layouts[i] == current) { idx = i; break; }
    HKL nextHkl = layouts[(idx + 1) % count];

    AttachThreadInput(ownThread, targetThread, TRUE);
    ActivateKeyboardLayout(nextHkl, KLF_SETFORPROCESS);
    SendMessage(hwnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)nextHkl);
    AttachThreadInput(ownThread, targetThread, FALSE);
    DefWindowProc(hwnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)nextHkl);

    std::cout << "=== Layout switched ===\n";
    std::cout << "  From : " << from << "\n";
    std::cout << "  To   : " << GetLayoutName(nextHkl) << "\n\n";
}
