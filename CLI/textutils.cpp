#include "textutils.h"
#include <cwctype>

// ===================== TEXT TRANSFORMATIONS =====================
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

// ===================== KEYBOARD LAYOUT REMAP =====================
// Notes for me on how it works:
//   Each character in the text was physically typed on layout A (wrong layout).
//   We find the virtual key (VK) that produces that character on layout A,
//   then ask layout B (the next layout) what character that same VK produces.
//   This way "<jkmifz <jxrf/" (typed in EN) becomes "Большая Бочка." (intended RU).
std::wstring RemapLayoutText(const std::wstring& text) {
    // Get all installed layouts
    int count = GetKeyboardLayoutList(0, nullptr);
    if (count < 2)
        return text; // Nothing to remap if only one layout exists

    std::vector<HKL> layouts(count);
    GetKeyboardLayoutList(count, layouts.data());

    // Determine the current layout of the active window
    HWND  hwnd         = GetForegroundWindow();
    DWORD targetThread = GetWindowThreadProcessId(hwnd, nullptr);
    HKL   currentHkl   = GetKeyboardLayout(targetThread);

    // Find the next layout to remap to
    int idx = 0;
    for (int i = 0; i < count; ++i)
        if (layouts[i] == currentHkl) { idx = i; break; }
    HKL targetHkl = layouts[(idx + 1) % count];

    std::wstring result;
    result.reserve(text.size());

    for (wchar_t wc : text) {
        SHORT vkAndShift = VkKeyScanExW(wc, currentHkl);

        if (vkAndShift == -1) {
            result += wc;
            continue;
        }

        BYTE vk    = LOBYTE(vkAndShift);
        BYTE shift = HIBYTE(vkAndShift);

        BYTE keyState[256] = {};
        if (shift & 1) keyState[VK_SHIFT] = 0x80; // Shift was held

        wchar_t buf[4] = {};
        int charCount = ToUnicodeEx(vk, 0, keyState, buf, 4, 0, targetHkl);

        if (charCount == 1)
            result += buf[0]; // Successfully remapped
        else
            result += wc;     // Could not remap — keep original character
    }

    return result;
}
