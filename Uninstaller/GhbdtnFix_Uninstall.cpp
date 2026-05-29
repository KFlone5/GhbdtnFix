#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#include "uninstall.h"
#include <windows.h>
#include <string>

// Must match the name used in the main app
const std::string APP_NAME = "GhbdtnFix";
const std::string EXE_NAME = "GhbdtnFix.exe";

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // Confirmation before doing anything
    int confirm = MessageBox(
        NULL,
        L"This will:\n\n"
        L"  \x2022 Remove GhbdtnFix from Windows startup\n"
        L"  \x2022 Close the running instance (if any)\n\n"
        L"Continue?",
        L"GhbdtnFix \x2014 Uninstall",
        MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2
    );

    if (confirm != IDYES)
        return 0;

    bool removedFromStartup = RemoveFromStartup(APP_NAME);
    bool killedProcess = KillRunningInstance(EXE_NAME);

    // Report result
    std::wstring msg;

    if (removedFromStartup && killedProcess) {
        msg = L"Done!\n\n"
              L"GhbdtnFix has been removed from startup\n"
              L"and the running instance was closed.";
    } else if (removedFromStartup && !killedProcess) {
        msg = L"Removed from startup.\n\n"
              L"No running instance of GhbdtnFix was found\n"
              L"(it may have already been closed).";
    } else if (!removedFromStartup && killedProcess) {
        msg = L"Running instance was closed.\n\n"
              L"Warning: could not remove the startup entry\n"
              L"(registry access denied or key not found).";
    } else {
        msg = L"Nothing changed.\n\n"
              L"GhbdtnFix was not in startup and no\n"
              L"running instance was found.";
    }

    MessageBox(NULL, msg.c_str(), L"GhbdtnFix \x2014 Uninstall", MB_ICONINFORMATION | MB_OK);
    return 0;
}
