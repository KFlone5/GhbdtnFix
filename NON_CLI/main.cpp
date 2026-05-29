#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#include "clipboard.h"
#include "textutils.h"
#include "startup.h"

// The name under which the program will appear in the registry
const std::string APP_NAME = "GhbdtnFix";

// ===================== HOTKEY IDs =====================
enum HotkeyID {
    HK_REMAP_LAYOUT = 1,  // Ctrl + 9
    HK_INVERT_CASE  = 2,  // Ctrl + ;
    HK_LOWERCASE    = 3,  // Ctrl + [
    HK_UPPERCASE    = 4,  // Ctrl + ]
    HK_REMOVE_SPACES= 5,  // Ctrl + 0
    HK_TYPE_CLIP    = 6,  // Ctrl + 1
    HK_EXIT         = 7,  // Ctrl + Alt + Q
    HK_PAUSE        = 8,  // Ctrl + Alt + P  (toggle pause)
};

// PAUSE HELPERS
// Registers all functional hotkeys
static void RegisterFunctionalHotkeys() {
    RegisterHotKey(nullptr, HK_REMAP_LAYOUT,  MOD_CONTROL,           '9');
    RegisterHotKey(nullptr, HK_INVERT_CASE,   MOD_CONTROL,           VK_OEM_1);
    RegisterHotKey(nullptr, HK_LOWERCASE,     MOD_CONTROL,           VK_OEM_4);
    RegisterHotKey(nullptr, HK_UPPERCASE,     MOD_CONTROL,           VK_OEM_6);
    RegisterHotKey(nullptr, HK_REMOVE_SPACES, MOD_CONTROL,           '0');
    RegisterHotKey(nullptr, HK_TYPE_CLIP,     MOD_CONTROL,           '1');
}

static void UnregisterFunctionalHotkeys() {
    UnregisterHotKey(nullptr, HK_REMAP_LAYOUT);
    UnregisterHotKey(nullptr, HK_INVERT_CASE);
    UnregisterHotKey(nullptr, HK_LOWERCASE);
    UnregisterHotKey(nullptr, HK_UPPERCASE);
    UnregisterHotKey(nullptr, HK_REMOVE_SPACES);
    UnregisterHotKey(nullptr, HK_TYPE_CLIP);
}

// Shows a brief balloon shape notification via a hidden tray icon
static void ShowTrayNotification(const wchar_t* title, const wchar_t* msg) {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd   = GetForegroundWindow();
    nid.uID    = 0xF1; // arbitrary unique ID
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO;
    nid.uTimeout    = 2000;
    wcsncpy_s(nid.szInfoTitle, title, 64);
    wcsncpy_s(nid.szInfo,      msg,   256);

    // Add a temporary icon just to show the balloon then remove
    nid.uFlags |= NIF_ICON | NIF_MESSAGE;
    nid.hIcon   = LoadIcon(nullptr, IDI_APPLICATION);
    nid.uCallbackMessage = WM_USER + 1;

    Shell_NotifyIconW(NIM_ADD,    &nid);
    Shell_NotifyIconW(NIM_MODIFY, &nid);

    Sleep(100); // Give Shell a moment to display the balloon

    Shell_NotifyIconW(NIM_DELETE, &nid);
}

// MAIN
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    std::string currentPath = GetExecutablePath();

    if (!IsInStartup(APP_NAME)) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)L"GhbdtnFix has been already started!\nWant to add a program to startup?\nThis will allow it to launch automatically when Windows starts.",
            (LPCWSTR)L"Startup",
            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1
        );

        if (msgboxID == IDYES) {
            AddToStartup(APP_NAME, currentPath);
        }
    }
    else {
        AddToStartup(APP_NAME, currentPath);
    }

    // Functional hotkeys (can be paused)
    RegisterFunctionalHotkeys();

    // System hotkeys (always active)
    // Ctrl + Alt + Q  - Exit
    RegisterHotKey(nullptr, HK_EXIT,  MOD_CONTROL | MOD_ALT, 'Q');
    // Ctrl + Alt + P  - Toggle pause
    RegisterHotKey(nullptr, HK_PAUSE, MOD_CONTROL | MOD_ALT, 'P');

    bool paused = false;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {

            // Exit (always works)
            if (msg.wParam == HK_EXIT)
                break;

            // Toggle pause (always works)
            if (msg.wParam == HK_PAUSE) {
                paused = !paused;
                if (paused) {
                    UnregisterFunctionalHotkeys();
                    ShowTrayNotification(
                        L"GhbdtnFix \x2014 Paused",
                        L"All hotkeys are disabled.\nPress Ctrl+Alt+P to resume."
                    );
                } else {
                    RegisterFunctionalHotkeys();
                    ShowTrayNotification(
                        L"GhbdtnFix \x2014 Active",
                        L"Hotkeys are enabled again."
                    );
                }
                continue;
            }

            // All other hotkeys — ignored while paused and unregistered
            if (paused)
                continue;

            // Save clipboard before we overwrite it with Ctrl+C
            std::wstring previousClipboard = GetClipboardTextW();

            Sleep(50);
            SendCtrlC();
            Sleep(100);

            std::wstring text = GetClipboardTextW();
            if (text.empty())
                continue;

            if (msg.wParam == HK_REMAP_LAYOUT) {
                TypeText(RemapLayoutText(text));
                SwitchToNextLayout();
            }
            else if (msg.wParam == HK_INVERT_CASE)
                TypeText(InvertCase(text));
            else if (msg.wParam == HK_LOWERCASE)
                TypeText(LowerCase(text));
            else if (msg.wParam == HK_UPPERCASE)
                TypeText(UpperCase(text));
            else if (msg.wParam == HK_REMOVE_SPACES)
                TypeText(RemoveSpaces(text));
            else if (msg.wParam == HK_TYPE_CLIP)
                TypeText(RemoveSpaces(text));

            // Restore clipboard to what it was before (Text only)
            SetClipboardTextW(previousClipboard);
        }
    }

    UnregisterFunctionalHotkeys();
    UnregisterHotKey(nullptr, HK_EXIT);
    UnregisterHotKey(nullptr, HK_PAUSE);

    return 0;
}
