#include "overlay.h"
#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <dxgi.h>

#include "../features/core.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
#include "Fonts/RobotoMedium.h"
#include "Fonts/RobotoRegular.h"
#include "Fonts/weaponIcons.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dxgi.lib")

typedef BOOL(WINAPI* SetWindowDisplayAffinity_t)(HWND, DWORD);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


__forceinline structures::Vector3 overlay::WorldToScreen(const structures::Vector3& point) noexcept {
    const auto& matrix = vars::viewMatrix.Matrix;

    __m128 vec = _mm_set_ps(1.f, point.z, point.y, point.x);
    __m128 row1 = _mm_set_ps(matrix[0][3], matrix[0][2], matrix[0][1], matrix[0][0]);
    __m128 row2 = _mm_set_ps(matrix[1][3], matrix[1][2], matrix[1][1], matrix[1][0]);
    __m128 row4 = _mm_set_ps(matrix[3][3], matrix[3][2], matrix[3][1], matrix[3][0]);

    __m128 dot1 = _mm_dp_ps(vec, row1, 0xFF);
    __m128 dot2 = _mm_dp_ps(vec, row2, 0xFF);
    __m128 dot4 = _mm_dp_ps(vec, row4, 0xFF);

    float _x = _mm_cvtss_f32(dot1);
    float _y = _mm_cvtss_f32(dot2);
    float w = _mm_cvtss_f32(dot4);

    if (w < 0.001f) {
        return structures::Vector3();
    }

    const float inv_w = 1.0f / w;
    _x *= inv_w;
    _y *= inv_w;

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float half_w = display.x * 0.5f;
    const float half_h = display.y * 0.5f;

    return structures::Vector3(
        half_w + (_x * half_w),
        half_h - (_y * half_h),
        w
    );
}

void ApplyImGuiStyle()
{
    ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.043f, 0.043f, 0.043f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.043f, 0.043f, 0.043f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.043f, 0.043f, 0.043f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.012f, 0.91f, 0.957f, 1.f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.043f, 0.043f, 0.043f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.043f, 0.043f, 0.043f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.012f, 0.91f, 0.957f, 1.f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.043f, 0.043f, 0.043f, 0.73f);
    style.TabRounding = 8.f;
    style.FrameRounding = 8.f;
    style.GrabRounding = 8.f;
    style.WindowRounding = 8.f;
    style.PopupRounding = 8.f;
    style.FrameRounding = 3.0f;
    style.WindowPadding = ImVec2(8, 20);
    style.WindowBorderSize = 0;
    style.ScrollbarSize = 1;
    style.GrabMinSize = 8;
    style.ChildBorderSize = 0;
    style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
}

void overlay::render() {
    static HWND gameWindow = FindWindowA(xorstr_("SDL_app"), xorstr_("Counter-Strike 2"));
    if (!gameWindow) return;

    static const WNDCLASSEXW wc = {
        sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        xorstr_(L"PicoDriver"), nullptr
    };
    static bool initialized = false;
    if (!initialized) {
        RegisterClassExW(&wc);
        initialized = true;
    }

    static HWND overlayWindow = nullptr;
    static RECT lastGameRect{};
    RECT currentGameRect;

    if (!GetClientRect(gameWindow, &currentGameRect)) return;

    if (!overlayWindow || memcmp(&lastGameRect, &currentGameRect, sizeof(RECT)) != 0) {
        POINT gamePos = { currentGameRect.left, currentGameRect.top };
        ClientToScreen(gameWindow, &gamePos);

        if (!overlayWindow) {
            overlayWindow = CreateWindowExW(
                WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
                wc.lpszClassName, xorstr_(L"PicoDriver"), WS_POPUP,
                gamePos.x - 1, gamePos.y,
                currentGameRect.right - currentGameRect.left,
                currentGameRect.bottom - currentGameRect.top,
                nullptr, nullptr, wc.hInstance, nullptr
            );

            if (!overlayWindow) return;

            SetLayeredWindowAttributes(overlayWindow, RGB(0, 0, 0), 255, LWA_ALPHA);

            SetWindowLong(overlayWindow, GWL_EXSTYLE,
                WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

            static const auto SetWindowDisplayAffinity =
                (SetWindowDisplayAffinity_t)GetProcAddress(GetModuleHandleA(xorstr_("user32.dll")), xorstr_("SetWindowDisplayAffinity"));
            if (SetWindowDisplayAffinity) {
                SetWindowDisplayAffinity(overlayWindow, WDA_EXCLUDEFROMCAPTURE);
            }

            static const BOOL disabled = TRUE;
            DwmSetWindowAttribute(overlayWindow, DWMWA_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(BOOL));
            static const MARGINS margin = { -1 };
            DwmExtendFrameIntoClientArea(overlayWindow, &margin);

            if (!CreateDeviceD3D(overlayWindow)) {
                CleanupDeviceD3D();
                ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
                return;
            }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;
            io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

            if (!fonts::normal) {
                io.FontDefault = fonts::normal = io.Fonts->AddFontFromMemoryTTF(
                    (void*)Fonts::RobotoRegular, Fonts::RobotoRegular_size, 15.0f);
                fonts::Title = io.Fonts->AddFontFromMemoryTTF(
                    (void*)Fonts::RobotoMedium, Fonts::RobotoMedium_size, 16.f);
                fonts::icons = io.Fonts->AddFontFromMemoryTTF(
                    (void*)Fonts::WeaponIcons, Fonts::WeaponIcons_size, 14.0f);
            }

            ImGui_ImplWin32_Init(overlayWindow);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            ApplyImGuiStyle();
            ShowWindow(overlayWindow, SW_SHOWDEFAULT);
            UpdateWindow(overlayWindow);
        }
        else {
            SetWindowPos(overlayWindow, HWND_TOPMOST, gamePos.x, gamePos.y,
                currentGameRect.right - currentGameRect.left,
                currentGameRect.bottom - currentGameRect.top,
                SWP_NOREDRAW);
        }

        lastGameRect = currentGameRect;
    }

    static LARGE_INTEGER frequency, lastFrameTime;
    static bool firstFrame = true;
    if (firstFrame) {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastFrameTime);
        firstFrame = false;
    }

    const float targetFrametime = 1.0f / vars::maxFps;
    LARGE_INTEGER currentTime;

    QueryPerformanceCounter(&currentTime);
    const float deltaTime = float(currentTime.QuadPart - lastFrameTime.QuadPart) / frequency.QuadPart;

    if (deltaTime < targetFrametime) {
        YieldProcessor();
        return;
    }

    lastFrameTime = currentTime;

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) return;
    }

    if (GetAsyncKeyState(VK_HOME) & 1) {
        menuEnabled = !menuEnabled;
        SetWindowLong(overlayWindow, GWL_EXSTYLE,
            menuEnabled ? WS_EX_TOPMOST : (WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED));
    }
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    Feature::Main();

    if (menuEnabled) {
        drawMenu();
    }

    ImGui::Render();

    static const ImVec4 clearColor(0.f, 0.f, 0.f, 0.0f);
    const float clearColorWithAlpha[4] = {
        clearColor.x * clearColor.w,
        clearColor.y * clearColor.w,
        clearColor.z * clearColor.w,
        clearColor.w
    };

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColorWithAlpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
}

bool GetBestAdapter(IDXGIAdapter1** ppAdapter) {
    IDXGIFactory1* factory = nullptr;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return false;

    IDXGIAdapter1* adapter = nullptr;
    IDXGIAdapter1* bestAdapter = nullptr;
    SIZE_T bestVideoMemory = 0;

    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            adapter->Release();
            continue;
        }

        if (desc.DedicatedVideoMemory > bestVideoMemory) {
            if (bestAdapter) bestAdapter->Release();
            bestAdapter = adapter;
            bestVideoMemory = desc.DedicatedVideoMemory;

            char adapterName[128];
            size_t convertedChars = 0;
            wcstombs_s(&convertedChars, adapterName, sizeof(adapterName), desc.Description, _TRUNCATE);
        }
        else {
            adapter->Release();
        }
    }

    factory->Release();
    *ppAdapter = bestAdapter;
    return bestAdapter != nullptr;

}

bool CreateDeviceD3D(HWND hWnd)
{
    static IDXGIAdapter1* adapter = nullptr;
    if (!adapter && !GetBestAdapter(&adapter)) return false;
    constexpr UINT createDeviceFlags =
        D3D11_CREATE_DEVICE_BGRA_SUPPORT |
        D3D11_CREATE_DEVICE_SINGLETHREADED;

    DXGI_SWAP_CHAIN_DESC sd{};
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    constexpr D3D_FEATURE_LEVEL featureLevels[] = {
         D3D_FEATURE_LEVEL_11_0,
         D3D_FEATURE_LEVEL_10_0
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        adapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        createDeviceFlags,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        &featureLevel,
        &g_pd3dDeviceContext
    );

    adapter->Release();

    if (FAILED(res)) {
        return false;
    }

    IDXGIDevice1* dxgiDevice = nullptr;
    if (SUCCEEDED(g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))) {
        dxgiDevice->SetMaximumFrameLatency(1);
        dxgiDevice->Release();
    }

    CreateRenderTarget();

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}