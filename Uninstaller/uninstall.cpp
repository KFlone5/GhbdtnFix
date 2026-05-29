#include "uninstall.h"
#include <tlhelp32.h>

// REMOVE FROM STARTUP
bool RemoveFromStartup(const std::string& appName) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    LONG result = RegDeleteValueA(hKey, appName.c_str());
    RegCloseKey(hKey);

    // ERROR_FILE_NOT_FOUND means it was already absent which still counts as success
    return (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND);
}

// KILL RUNNING INSTANCE
bool KillRunningInstance(const std::string& exeName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32 pe = {};
    pe.dwSize = sizeof(pe);

    bool killed = false;

    if (Process32First(hSnap, &pe)) {
        do {
            char procName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1,
                procName, sizeof(procName), nullptr, nullptr);

            if (_stricmp(procName, exeName.c_str()) == 0) {
                // Skip ourselves
                if (pe.th32ProcessID == GetCurrentProcessId())
                    continue;

                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProc) {
                    if (TerminateProcess(hProc, 0))
                        killed = true;
                    CloseHandle(hProc);
                }
            }
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return killed;
}
