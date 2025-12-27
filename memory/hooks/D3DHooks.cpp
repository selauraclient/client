#include "D3DHooks.hpp"

static winrt::com_ptr<ID3D11Device> device;
static winrt::com_ptr<ID3D11DeviceContext> device_ctx;
static winrt::com_ptr<ID3D11Texture2D> back_buffer;
static winrt::com_ptr<ID3D11RenderTargetView> main_rtv;

static bool d3d_init = false;

HRESULT selaura::detour<&IDXGISwapChain::Present>::hk(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {
    static bool imgui_init = false;
    if (!d3d_init) {
        if (!imgui_init) {
            thisptr->GetDevice(__uuidof(device), device.put_void());
            device->GetImmediateContext(device_ctx.put());

            ImGui::CreateContext();
            ImGui_ImplWin32_Init(GetForegroundWindow());
            ImGui_ImplDX11_Init(device.get(), device_ctx.get());

            imgui_init = true;
        }

        if (SUCCEEDED(thisptr->GetBuffer(0, __uuidof(ID3D11Texture2D), back_buffer.put_void()))) {
            device->CreateRenderTargetView(back_buffer.get(), NULL, main_rtv.put());
        }

        d3d_init = true;
    }

    if (!main_rtv) return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    auto dl = ImGui::GetForegroundDrawList();
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 3, ImVec2(10, 10),
            ImColor(255, 255, 255, 255),"i love selaura client!");

    ImGui::Render();

    ID3D11RenderTargetView* rtv_ptr = main_rtv.get();
    device_ctx->OMSetRenderTargets(1, &rtv_ptr, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);
}

HRESULT selaura::detour<&IDXGISwapChain::ResizeBuffers>::hk(IDXGISwapChain* thisptr, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    if (device_ctx) {
        ID3D11RenderTargetView* null_rtv = nullptr;
        device_ctx->OMSetRenderTargets(0, &null_rtv, nullptr);
        device_ctx->ClearState();
        device_ctx->Flush();
    }

    main_rtv = nullptr;
    back_buffer = nullptr;

    d3d_init = false;

    return selaura::hook<&IDXGISwapChain::ResizeBuffers>::call(thisptr, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

void selaura::detour<&bgfx::d3d11::RenderContextD3D11::submit>::hk(bgfx::d3d11::RenderContextD3D11* thisptr, void* a1, void* a2, void* a3) {
    selaura::hook<&bgfx::d3d11::RenderContextD3D11::submit>::call(thisptr, a1, a2, a3);

    static bool once = false;
    if (!once) {
        selaura::hook<&IDXGISwapChain::Present>::enable(thisptr->mSwapChain);
        selaura::hook<&IDXGISwapChain::ResizeBuffers>::enable(thisptr->mSwapChain);
        once = true;
    }
}