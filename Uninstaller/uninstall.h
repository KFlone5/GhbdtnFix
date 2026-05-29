#pragma once
#include <windows.h>
#include <string>

bool RemoveFromStartup(const std::string& appName);
bool KillRunningInstance(const std::string& exeName);
