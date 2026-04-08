#include "startup.h"

bool IsInStartup(const std::string& appName) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    char value[512];
    DWORD value_length = sizeof(value);

    LONG result = RegQueryValueExA(
        hKey,
        appName.c_str(),
        NULL,
        NULL,
        (LPBYTE)value,
        &value_length
    );

    RegCloseKey(hKey);

    return (result == ERROR_SUCCESS);
}

bool AddToStartup(const std::string& appName, const std::string& path) {
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    LONG result = RegSetValueExA(
        hKey,
        appName.c_str(),
        0,
        REG_SZ,
        (BYTE*)path.c_str(),
        path.size() + 1
    );

    RegCloseKey(hKey);

    return (result == ERROR_SUCCESS);
}

std::string GetExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
}