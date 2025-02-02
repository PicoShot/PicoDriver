#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "Utils/driver.h"
#include <d3d11.h>
#include "imgui/imgui.h"
#include "overlay/overlay.h"
#include "config/updater.h"
#include "config/config.h"
#include <fcntl.h>
#include <io.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

updater::OffsetUpdater update;

void SetupConsole() {
    AllocConsole();

    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    int fdStdout = _open_osfhandle((intptr_t)hStdout, _O_TEXT);
    FILE* fpStdout = _fdopen(fdStdout, "w");
    freopen_s(&fpStdout, "CONOUT$", "w", stdout);

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    int fdStdin = _open_osfhandle((intptr_t)hStdin, _O_TEXT);
    FILE* fpStdin = _fdopen(fdStdin, "r");
    freopen_s(&fpStdin, "CONIN$", "r", stdin);

    HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);
    int fdStderr = _open_osfhandle((intptr_t)hStderr, _O_TEXT);
    FILE* fpStderr = _fdopen(fdStderr, "w");
    freopen_s(&fpStderr, "CONOUT$", "w", stderr);

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
	//SetupConsole();
    
    config::LoadConfig();

    if (!update.UpdateOffsets())
    {
        return 1;
    }

    driver::AttachToProcess(xorstr_(L"cs2.exe"));
    vars::clientBase = driver::GetModuleBaseByName(xorstr_(L"client.dll"));

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        overlay::render();
    }
    return 0;
}
