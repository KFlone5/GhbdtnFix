#pragma once
#include <windows.h>
#include <string>

bool IsInStartup(const std::string& appName);
bool AddToStartup(const std::string& appName, const std::string& path);
std::string GetExecutablePath();