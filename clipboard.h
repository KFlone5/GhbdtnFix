#pragma once
#include <windows.h>
#include <string>

void SendCtrlC();
std::wstring GetClipboardTextW();
void TypeText(const std::wstring& text);
