#include "textutils.h"
#include <cwctype>

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
