#pragma once
#include <windows.h>
#include <string>
#include <vector>

// ===================== CLIPBOARD & INPUT =====================
void SendCtrlC();
std::wstring GetClipboardTextW();
void TypeText(const std::wstring& text);

// ===================== KEYBOARD LAYOUT =====================
std::string GetLayoutName(HKL hkl);
void ShowInstalledLayouts();
void ShowCurrentLayout();
void SwitchToNextLayout();
