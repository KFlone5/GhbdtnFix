#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(lib, "shell32.lib")
#include "resource.h"
#include "clipboard.h"
#include "textutils.h"
#include "startup.h"
#include <shellapi.h>

const std::string APP_NAME = "GhbdtnFix";

#define WM_TRAY      (WM_USER + 1)
#define IDM_PAUSE    100
#define IDM_STARTUP  101
#define IDM_EXIT     102
#define TRAY_UID     1

enum HotkeyID {
    HK_REMAP_LAYOUT  = 1,  // Ctrl + 9
    HK_INVERT_CASE   = 2,  // Ctrl + ;
    HK_LOWERCASE     = 3,  // Ctrl + [
    HK_UPPERCASE     = 4,  // Ctrl + ]
    HK_REMOVE_SPACES = 5,  // Ctrl + 0
    HK_TYPE_CLIP     = 6,  // Ctrl + 1
    HK_EXIT          = 7,  // Ctrl + Alt + Q
    HK_PAUSE         = 8,  // Ctrl + Alt + P
};

static HINSTANCE       g_hInst      = nullptr;
static HWND            g_hwnd       = nullptr;
static NOTIFYICONDATAW g_nid        = {};
static bool            g_paused     = false;
static HICON           g_iconColor  = nullptr;
static HICON           g_iconGrey   = nullptr;

// Creates a greyscale copy of an icon (used for the paused state)
static HICON MakeGreyIcon(HICON hSrc) {
    ICONINFO ii = {};
    if (!GetIconInfo(hSrc, &ii)) return nullptr;

    BITMAP bm = {};
    GetObject(ii.hbmColor, sizeof(bm), &bm);
    int w = bm.bmWidth;
    int h = abs(bm.bmHeight);
    DeleteObject(ii.hbmColor);
    DeleteObject(ii.hbmMask);

    HDC     hdcScr = GetDC(nullptr);
    HDC     hdcMem = CreateCompatibleDC(hdcScr);

    BITMAPINFO bi             = {};
    bi.bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth      = w;
    bi.bmiHeader.biHeight     = -h;
    bi.bmiHeader.biPlanes     = 1;
    bi.bmiHeader.biBitCount   = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    DWORD*  pPx = nullptr;
    HBITMAP hbm = CreateDIBSection(hdcScr, &bi, DIB_RGB_COLORS, (void**)&pPx, nullptr, 0);
    HBITMAP old = (HBITMAP)SelectObject(hdcMem, hbm);

    memset(pPx, 0, w * h * 4);
    DrawIconEx(hdcMem, 0, 0, hSrc, w, h, 0, nullptr, DI_NORMAL);
    GdiFlush();

    // BGRA -> greyscale
    for (int i = 0; i < w * h; i++) {
        DWORD px = pPx[i];
        BYTE  b  = (BYTE)(px);
        BYTE  g  = (BYTE)(px >> 8);
        BYTE  r  = (BYTE)(px >> 16);
        BYTE  a  = (BYTE)(px >> 24);
        BYTE  gr = (BYTE)(0.299f * r + 0.587f * g + 0.114f * b);
        pPx[i]   = ((DWORD)a << 24) | ((DWORD)gr << 16) | ((DWORD)gr << 8) | gr;
    }

    SelectObject(hdcMem, old);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScr);

    // All black mask or alpha channel controls transparency
    HDC     hdcMsk = CreateCompatibleDC(nullptr);
    HBITMAP hMsk   = CreateBitmap(w, h, 1, 1, nullptr);
    HBITMAP oldMsk = (HBITMAP)SelectObject(hdcMsk, hMsk);
    PatBlt(hdcMsk, 0, 0, w, h, BLACKNESS);
    SelectObject(hdcMsk, oldMsk);
    DeleteDC(hdcMsk);

    ICONINFO iiNew = { TRUE, 0, 0, hMsk, hbm };
    HICON    hGrey = CreateIconIndirect(&iiNew);

    DeleteObject(hbm);
    DeleteObject(hMsk);
    return hGrey;
}

// Hotkeys
static void RegisterFunctionalHotkeys() {
    RegisterHotKey(g_hwnd, HK_REMAP_LAYOUT,  MOD_CONTROL, '9');
    RegisterHotKey(g_hwnd, HK_INVERT_CASE,   MOD_CONTROL, VK_OEM_1);
    RegisterHotKey(g_hwnd, HK_LOWERCASE,     MOD_CONTROL, VK_OEM_4);
    RegisterHotKey(g_hwnd, HK_UPPERCASE,     MOD_CONTROL, VK_OEM_6);
    RegisterHotKey(g_hwnd, HK_REMOVE_SPACES, MOD_CONTROL, '0');
    RegisterHotKey(g_hwnd, HK_TYPE_CLIP,     MOD_CONTROL, '1');
}

static void UnregisterFunctionalHotkeys() {
    UnregisterHotKey(g_hwnd, HK_REMAP_LAYOUT);
    UnregisterHotKey(g_hwnd, HK_INVERT_CASE);
    UnregisterHotKey(g_hwnd, HK_LOWERCASE);
    UnregisterHotKey(g_hwnd, HK_UPPERCASE);
    UnregisterHotKey(g_hwnd, HK_REMOVE_SPACES);
    UnregisterHotKey(g_hwnd, HK_TYPE_CLIP);
}

// Tray
static void UpdateTrayIcon() {
    g_nid.hIcon = g_paused ? g_iconGrey : g_iconColor;
    wcscpy_s(g_nid.szTip, g_paused ? L"GhbdtnFix \x2014 Paused" : L"GhbdtnFix \x2014 Active");
    Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}

static void ShowBalloon(const wchar_t* title, const wchar_t* text) {
    NOTIFYICONDATAW n = g_nid;
    n.uFlags          = NIF_INFO;
    n.dwInfoFlags     = NIIF_INFO | NIIF_NOSOUND;
    n.uTimeout        = 5000;
    wcsncpy_s(n.szInfoTitle, title, 64);
    wcsncpy_s(n.szInfo,      text,  256);
    Shell_NotifyIconW(NIM_MODIFY, &n);
}

static void ShowContextMenu() {
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(g_hwnd); // required for the menu to close on click-away

    bool inStartup = IsInStartup(APP_NAME);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, IDM_PAUSE,
                g_paused ? L"  Resume  (Ctrl+Alt+P)"
                         : L"  Pause  (Ctrl+Alt+P)");
    AppendMenuW(hMenu, MF_STRING | (inStartup ? MF_CHECKED : MF_UNCHECKED),
                IDM_STARTUP, L"  Launch at startup");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"  Exit  (Ctrl+Alt+Q)");

    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, g_hwnd, nullptr);
    DestroyMenu(hMenu);
}

static void TogglePause() {
    g_paused = !g_paused;
    if (g_paused) {
        UnregisterFunctionalHotkeys();
        UpdateTrayIcon();
        ShowBalloon(L"GhbdtnFix \x2014 Paused",
                    L"Hotkeys disabled.\nCtrl+Alt+P or tray icon to resume.");
    } else {
        RegisterFunctionalHotkeys();
        UpdateTrayIcon();
        ShowBalloon(L"GhbdtnFix \x2014 Active", L"Hotkeys are enabled again.");
    }
}

static void ToggleStartup() {
    std::string path = GetExecutablePath();
    if (IsInStartup(APP_NAME)) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            RegDeleteValueA(hKey, APP_NAME.c_str());
            RegCloseKey(hKey);
        }
        ShowBalloon(L"GhbdtnFix \x2014 Startup", L"Removed from startup.");
    } else {
        AddToStartup(APP_NAME, path);
        ShowBalloon(L"GhbdtnFix \x2014 Startup", L"Added to startup.");
    }
}

// Window proc
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_TRAY:
        if (lp == WM_RBUTTONUP)     ShowContextMenu();
        if (lp == WM_LBUTTONDBLCLK) TogglePause();
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDM_PAUSE:   TogglePause();        break;
        case IDM_STARTUP: ToggleStartup();      break;
        case IDM_EXIT:    PostQuitMessage(0);   break;
        }
        return 0;

    case WM_HOTKEY:
        if (wp == HK_EXIT)  { PostQuitMessage(0); return 0; }
        if (wp == HK_PAUSE) { TogglePause();      return 0; }
        if (g_paused)         return 0;
        {
            std::wstring prev = GetClipboardTextW();
            Sleep(50);
            SendCtrlC();
            Sleep(100);
            std::wstring text = GetClipboardTextW();
            if (text.empty()) return 0;

            if      (wp == HK_REMAP_LAYOUT)  { TypeText(RemapLayoutText(text)); SwitchToNextLayout(); }
            else if (wp == HK_INVERT_CASE)     TypeText(InvertCase(text));
            else if (wp == HK_LOWERCASE)       TypeText(LowerCase(text));
            else if (wp == HK_UPPERCASE)       TypeText(UpperCase(text));
            else if (wp == HK_REMOVE_SPACES)   TypeText(RemoveSpaces(text));
            else if (wp == HK_TYPE_CLIP)       TypeText(RemoveSpaces(text));

            SetClipboardTextW(prev);
        }
        return 0;

    case WM_DESTROY:
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        if (g_iconGrey) DestroyIcon(g_iconGrey);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// MAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    g_hInst = hInstance;

    // Hidden message-only window (needed for tray + hotkeys)
    WNDCLASSW wc      = {};
    wc.lpfnWndProc    = WndProc;
    wc.hInstance      = hInstance;
    wc.lpszClassName  = L"GhbdtnFixWnd";
    RegisterClassW(&wc);

    g_hwnd = CreateWindowExW(0, L"GhbdtnFixWnd", L"GhbdtnFix",
                              0, 0, 0, 0, 0,
                              HWND_MESSAGE, nullptr, hInstance, nullptr);

    // Startup check
    std::string path = GetExecutablePath();
    if (!IsInStartup(APP_NAME)) {
        int ans = MessageBox(nullptr,
            L"GhbdtnFix is not in startup.\nAdd it so it launches automatically with Windows?",
            L"GhbdtnFix \x2014 Startup", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
        if (ans == IDYES)
            AddToStartup(APP_NAME, path);
    } else {
        AddToStartup(APP_NAME, path); // keeps the path up to date if the exe was moved
    }

    // Load icons (colored + greyscale for paused state)
    g_iconColor = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    g_iconGrey  = MakeGreyIcon(g_iconColor);

    // Add persistent tray icon
    g_nid.cbSize           = sizeof(g_nid);
    g_nid.hWnd             = g_hwnd;
    g_nid.uID              = TRAY_UID;
    g_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon            = g_iconColor;
    wcscpy_s(g_nid.szTip, L"GhbdtnFix \x2014 Active");
    Shell_NotifyIconW(NIM_ADD, &g_nid);

    // Register hotkeys
    RegisterFunctionalHotkeys();
    RegisterHotKey(g_hwnd, HK_EXIT,  MOD_CONTROL | MOD_ALT, 'Q');
    RegisterHotKey(g_hwnd, HK_PAUSE, MOD_CONTROL | MOD_ALT, 'P');

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterFunctionalHotkeys();
    UnregisterHotKey(g_hwnd, HK_EXIT);
    UnregisterHotKey(g_hwnd, HK_PAUSE);

    return 0;
}
