#include "custom_imgui/custom_imgui.hpp"

#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <d3d11.h>
#include <tchar.h>

static ID3D11Device* gDevice = nullptr;
static ID3D11DeviceContext* gDeviceContext = nullptr;
static IDXGISwapChain* gSwapChain = nullptr;
static bool gSwapChainOccluded = false;
static UINT gResizeWidth = 0;
static UINT gResizeHeight = 0;
static ID3D11RenderTargetView* gMainRenderTargetView = nullptr;

bool createDeviceD3D(HWND hwnd);
void cleanupDeviceD3D();
void createRenderTarget();
void cleanupRenderTarget();
LRESULT WINAPI wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void drawAnimationSmokeTest()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(520.0f, 320.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("CustomImGui / ImAnim smoke test");

    static bool expanded = true;
    ImGui::Checkbox("Animate width", &expanded);
    const float targetWidth = expanded ? 420.0f : 120.0f;
    const float width = iam_tween_float(ImGui::GetID("animated-width"), ImGui::GetID("animated-width-channel"), targetWidth, 0.28f,
                                        iam_ease_preset(iam_ease_out_cubic), iam_policy_crossfade, io.DeltaTime, targetWidth);

    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + width, p.y + 42.0f), IM_COL32(66, 150, 245, 255), 8.0f);
    dl->AddText(ImVec2(p.x + 14.0f, p.y + 12.0f), IM_COL32_WHITE, "Animated by ImAnim");
    ImGui::Dummy(ImVec2(width, 52.0f));

    ImGui::Separator();
    ImGui::Text("Dear ImGui: %s", IMGUI_VERSION);
    ImGui::Text("ImAnim: %s", IMANIM_VERSION);
    ImGui::Text("Delta: %.3f ms", io.DeltaTime * 1000.0f);
    ImGui::End();
}

int main(int, char**)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    const float mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC,         wndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr,
                      nullptr,    L"CustomImGuiTest", nullptr};
    ::RegisterClassExW(&wc);
    HWND hwnd =
        ::CreateWindowW(wc.lpszClassName, L"CustomImGui DX11 Test", WS_OVERLAPPEDWINDOW, 100, 100, static_cast<int>(1120 * mainScale),
                        static_cast<int>(720 * mainScale), nullptr, nullptr, wc.hInstance, nullptr);

    if (!createDeviceD3D(hwnd)) {
        cleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(mainScale);
    style.FontScaleDpi = mainScale;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(gDevice, gDeviceContext);

    bool done = false;
    ImVec4 clearColor = ImVec4(0.09f, 0.09f, 0.10f, 1.0f);
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                done = true;
            }
        }
        if (done) {
            break;
        }

        if (gSwapChainOccluded && gSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            ::Sleep(10);
            continue;
        }
        gSwapChainOccluded = false;

        if (gResizeWidth != 0 && gResizeHeight != 0) {
            cleanupRenderTarget();
            gSwapChain->ResizeBuffers(0, gResizeWidth, gResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            gResizeWidth = 0;
            gResizeHeight = 0;
            createRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        CustomImGui::BeginAnimationFrame();

        drawAnimationSmokeTest();

        ImGui::Render();
        const float clearColorWithAlpha[4] = {clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w,
                                              clearColor.w};
        gDeviceContext->OMSetRenderTargets(1, &gMainRenderTargetView, nullptr);
        gDeviceContext->ClearRenderTargetView(gMainRenderTargetView, clearColorWithAlpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT hr = gSwapChain->Present(1, 0);
        gSwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

bool createDeviceD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2,
                                                D3D11_SDK_VERSION, &sd, &gSwapChain, &gDevice, &featureLevel, &gDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) {
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2,
                                            D3D11_SDK_VERSION, &sd, &gSwapChain, &gDevice, &featureLevel, &gDeviceContext);
    }
    if (res != S_OK) {
        return false;
    }

    createRenderTarget();
    return true;
}

void cleanupDeviceD3D()
{
    cleanupRenderTarget();
    if (gSwapChain) {
        gSwapChain->Release();
        gSwapChain = nullptr;
    }
    if (gDeviceContext) {
        gDeviceContext->Release();
        gDeviceContext = nullptr;
    }
    if (gDevice) {
        gDevice->Release();
        gDevice = nullptr;
    }
}

void createRenderTarget()
{
    ID3D11Texture2D* backBuffer = nullptr;
    gSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    gDevice->CreateRenderTargetView(backBuffer, nullptr, &gMainRenderTargetView);
    backBuffer->Release();
}

void cleanupRenderTarget()
{
    if (gMainRenderTargetView) {
        gMainRenderTargetView->Release();
        gMainRenderTargetView = nullptr;
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
        return true;
    }

    switch (msg) {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) {
            return 0;
        }
        gResizeWidth = static_cast<UINT>(LOWORD(lParam));
        gResizeHeight = static_cast<UINT>(HIWORD(lParam));
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) {
            return 0;
        }
        break;
    case WM_DESTROY: ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}
