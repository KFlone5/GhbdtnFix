#pragma once
#include <windows.h>
#include <string>
#include <vector>

// ===================== TEXT TRANSFORMATIONS =====================
std::wstring InvertCase(const std::wstring& text);
std::wstring LowerCase(const std::wstring& text);
std::wstring UpperCase(const std::wstring& text);
std::wstring RemoveSpaces(const std::wstring& text);

// ===================== KEYBOARD LAYOUT Change =====================
// Changes selected text to the next available layout
// Example: "<jkmifz <jxrf/" (typed in EN) -> "Большая Бочка." (intended RU)
std::wstring RemapLayoutText(const std::wstring& text);
