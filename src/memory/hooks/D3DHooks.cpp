#include <pch.hpp>

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include <core/service_manager.hpp>
#include <memory/detour.hpp>
#include <memory/hook.hpp>

#include <sdk/renderer/bgfx.hpp>

static winrt::com_ptr<ID3D11Device> device;
static winrt::com_ptr<ID3D11DeviceContext> device_ctx;
static winrt::com_ptr<ID3D11Texture2D> back_buffer;
static winrt::com_ptr<ID3D11RenderTargetView> main_rtv;

static winrt::com_ptr<ID3D11On12Device> d3d11on12_device;
static ID3D12CommandQueue* d3d12_queue = nullptr;
static winrt::com_ptr<ID3D11Resource> wrapped_back_buffer;

static WNDPROC wnd_proc = nullptr;
static bool is_d3d12 = false;
static bool d3d_init = false;
static bool imgui_init = false;
static bool hooks_init = false;

LOAD_RESOURCE(Poppins_msdf_png)
LOAD_RESOURCE(Poppins_msdf_json)

LOAD_RESOURCE(Minecraft_msdf_png)
LOAD_RESOURCE(Minecraft_msdf_json)


LOAD_RESOURCE(FontAwesome_msdf_png)
LOAD_RESOURCE(FontAwesome_msdf_json)

template <>
struct selaura::detour<&IDXGISwapChain::Present> {
    static HRESULT hk(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {
        if (!d3d_init) {
            winrt::com_ptr<ID3D12Device> d3d12_dev;
            HRESULT hr = thisptr->GetDevice(__uuidof(ID3D12Device), d3d12_dev.put_void());

            if (SUCCEEDED(hr)) {
                is_d3d12 = true;

                if (!d3d12_queue) return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);

                hr = D3D11On12CreateDevice(
                    d3d12_dev.get(), D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                    nullptr, 0, reinterpret_cast<IUnknown**>(&d3d12_queue), 1,
                    0, device.put(), device_ctx.put(), nullptr
                );

                if (FAILED(hr)) return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);

                device->QueryInterface(d3d11on12_device.put());
            } else {
                is_d3d12 = false;
                if (FAILED(thisptr->GetDevice(__uuidof(ID3D11Device), device.put_void()))) return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);

                device->GetImmediateContext(device_ctx.put());

                winrt::com_ptr<ID3D11Texture2D> pBackBuffer;
                thisptr->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.put()));
                device->CreateRenderTargetView(pBackBuffer.get(), nullptr, main_rtv.put());
            }

            if (!imgui_init && device) {
                ImGui::CreateContext();
                DXGI_SWAP_CHAIN_DESC desc{};
                thisptr->GetDesc(&desc);

                ImGui_ImplWin32_Init(desc.OutputWindow);
                ImGui_ImplDX11_Init(device.get(), device_ctx.get());

                {
                    ImGuiStyle& style = ImGui::GetStyle();
                    ImVec4* colors = style.Colors;

                    const ImVec4 base00 = ImVec4(0.086f, 0.086f, 0.086f, 1.00f); // #161616 (Main Background)
                    const ImVec4 base01 = ImVec4(0.149f, 0.149f, 0.149f, 1.00f); // #262626 (Secondary Background)
                    const ImVec4 base02 = ImVec4(0.224f, 0.224f, 0.224f, 1.00f); // #393939 (UI Elements / Hover)
                    const ImVec4 base03 = ImVec4(0.322f, 0.322f, 0.322f, 1.00f); // #525252 (Selection / Active)
                    const ImVec4 base04 = ImVec4(0.867f, 0.882f, 0.902f, 1.00f); // #dde1e6 (Subtext)
                    const ImVec4 base05 = ImVec4(0.949f, 0.957f, 0.973f, 1.00f); // #f2f4f8 (Main Text)
                    const ImVec4 base06 = ImVec4(1.000f, 1.000f, 1.000f, 1.00f); // #ffffff (Pure White)

                    // Accents
                    const ImVec4 base07 = ImVec4(0.031f, 0.741f, 0.729f, 1.00f); // #08bdba (Teal/Cyan)
                    const ImVec4 base08 = ImVec4(0.239f, 0.859f, 0.851f, 1.00f); // #3ddbd9 (Light Cyan)
                    const ImVec4 base09 = ImVec4(0.471f, 0.663f, 1.000f, 1.00f); // #78a9ff (Blue)
                    const ImVec4 base0A = ImVec4(0.933f, 0.325f, 0.588f, 1.00f); // #ee5396 (Pink)
                    const ImVec4 base0B = ImVec4(0.200f, 0.694f, 1.000f, 1.00f); // #33b1ff (Light Blue)
                    const ImVec4 base0C = ImVec4(1.000f, 0.494f, 0.714f, 1.00f); // #ff7eb6 (Light Pink)
                    const ImVec4 base0D = ImVec4(0.259f, 0.745f, 0.396f, 1.00f); // #42be65 (Green)
                    const ImVec4 base0E = ImVec4(0.745f, 0.584f, 1.000f, 1.00f); // #be95ff (Purple/Mauve)
                    const ImVec4 base0F = ImVec4(0.510f, 0.812f, 1.000f, 1.00f); // #82cfff (Sky Blue)

                    colors[ImGuiCol_Text]                 = base05;
                    colors[ImGuiCol_TextDisabled]         = base03;
                    colors[ImGuiCol_WindowBg]             = base00;
                    colors[ImGuiCol_ChildBg]              = base00;
                    colors[ImGuiCol_PopupBg]              = base01;
                    colors[ImGuiCol_Border]               = base02;
                    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

                    colors[ImGuiCol_FrameBg]              = base01;
                    colors[ImGuiCol_FrameBgHovered]       = base02;
                    colors[ImGuiCol_FrameBgActive]        = base03;

                    colors[ImGuiCol_TitleBg]              = base01;
                    colors[ImGuiCol_TitleBgActive]        = base02;
                    colors[ImGuiCol_TitleBgCollapsed]     = base00;
                    colors[ImGuiCol_MenuBarBg]            = base01;

                    colors[ImGuiCol_ScrollbarBg]          = base00;
                    colors[ImGuiCol_ScrollbarGrab]        = base02;
                    colors[ImGuiCol_ScrollbarGrabHovered] = base03;
                    colors[ImGuiCol_ScrollbarGrabActive]  = base07;

                    colors[ImGuiCol_CheckMark]            = base08;
                    colors[ImGuiCol_SliderGrab]           = base09;
                    colors[ImGuiCol_SliderGrabActive]     = base0B;

                    colors[ImGuiCol_Button]               = base01;
                    colors[ImGuiCol_ButtonHovered]        = base02;
                    colors[ImGuiCol_ButtonActive]         = base03;

                    colors[ImGuiCol_Header]               = base01;
                    colors[ImGuiCol_HeaderHovered]        = base02;
                    colors[ImGuiCol_HeaderActive]         = base03;

                    colors[ImGuiCol_Separator]            = base02;
                    colors[ImGuiCol_SeparatorHovered]     = base0E;
                    colors[ImGuiCol_SeparatorActive]      = base0E;

                    colors[ImGuiCol_ResizeGrip]           = ImVec4(0,0,0,0);
                    colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0,0,0,0);
                    colors[ImGuiCol_ResizeGripActive]     = ImVec4(0,0,0,0);

                    colors[ImGuiCol_Tab]                  = base01;
                    colors[ImGuiCol_TabHovered]           = base03;
                    colors[ImGuiCol_TabActive]            = base02;
                    colors[ImGuiCol_TabUnfocused]         = base01;
                    colors[ImGuiCol_TabUnfocusedActive]   = base02;

                    colors[ImGuiCol_PlotLines]            = base0E;
                    colors[ImGuiCol_PlotLinesHovered]     = base0A;
                    colors[ImGuiCol_PlotHistogram]        = base07;
                    colors[ImGuiCol_PlotHistogramHovered] = base08;

                    colors[ImGuiCol_TableHeaderBg]        = base01;
                    colors[ImGuiCol_TableBorderStrong]    = base02;
                    colors[ImGuiCol_TableBorderLight]     = base01;
                    colors[ImGuiCol_TableRowBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                    colors[ImGuiCol_TableRowBgAlt]        = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);

                    colors[ImGuiCol_TextSelectedBg]       = base03;
                    colors[ImGuiCol_DragDropTarget]       = base0D;
                    colors[ImGuiCol_NavHighlight]         = base0E;
                    colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);

                    style.WindowRounding    = 6.0f;
                    style.ChildRounding     = 6.0f;
                    style.FrameRounding     = 4.0f;
                    style.PopupRounding     = 4.0f;
                    style.ScrollbarRounding = 9.0f;
                    style.GrabRounding      = 4.0f;
                    style.TabRounding       = 4.0f;

                    style.WindowPadding     = ImVec2(8.0f, 8.0f);
                    style.FramePadding      = ImVec2(5.0f, 3.0f);
                    style.ItemSpacing       = ImVec2(8.0f, 4.0f);
                    style.ItemInnerSpacing  = ImVec2(4.0f, 4.0f);
                    style.IndentSpacing     = 21.0f;
                    style.ScrollbarSize     = 14.0f;
                    style.GrabMinSize       = 10.0f;

                    style.WindowBorderSize  = 1.0f;
                    style.ChildBorderSize   = 1.0f;
                    style.PopupBorderSize   = 1.0f;
                    style.FrameBorderSize   = 0.0f;
                    style.TabBorderSize     = 0.0f;
                }

                imgui_init = true;
            }
            d3d_init = true;
        }

        if (is_d3d12 && d3d11on12_device) {
            winrt::com_ptr<ID3D12Resource> d3d12_bb;

            UINT backBufferIdx = ((IDXGISwapChain3*)thisptr)->GetCurrentBackBufferIndex();
            HRESULT hr = thisptr->GetBuffer(backBufferIdx, IID_PPV_ARGS(d3d12_bb.put()));

            if (FAILED(hr)) {
                thisptr->GetBuffer(0, IID_PPV_ARGS(d3d12_bb.put()));
            }

            D3D11_RESOURCE_FLAGS rf = { D3D11_BIND_RENDER_TARGET };

            hr = d3d11on12_device->CreateWrappedResource(
                d3d12_bb.get(), &rf,
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_PRESENT,
                IID_PPV_ARGS(wrapped_back_buffer.put())
            );

            if (SUCCEEDED(hr)) {
                auto resource_ptr = wrapped_back_buffer.get();
                d3d11on12_device->AcquireWrappedResources(&resource_ptr, 1);
                device->CreateRenderTargetView(wrapped_back_buffer.get(), nullptr, main_rtv.put());
            }
        }

        if (main_rtv) {
            ID3D11RenderTargetView* rtv_ptr = main_rtv.get();
            device_ctx->OMSetRenderTargets(1, &rtv_ptr, nullptr);

            DXGI_SWAP_CHAIN_DESC desc;
            thisptr->GetDesc(&desc);

            static bool renderer_init = false;
            auto& renderer = selaura::get<selaura::renderer>();

            if (!renderer_init) {
                renderer.init(device.get());
                renderer_init = true;
            }

            renderer.set_font(
                GET_RESOURCE(Poppins_msdf_png),
                GET_RESOURCE(Poppins_msdf_json)
            );

            static auto last_time = std::chrono::high_resolution_clock::now();
            static float fps = 0.0f;
            static int frame_count = 0;
            static float accumulator = 0.0f;

            auto current_time = std::chrono::high_resolution_clock::now();
            float delta_time = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;
            accumulator += delta_time;
            frame_count++;

            if (accumulator >= 0.5f) {
                fps = static_cast<float>(frame_count) / accumulator;
                accumulator = 0.0f;
                frame_count = 0;
            }

            selaura::render_event ev{};

            ev.swapChain = thisptr;
            ev.screenWidth = desc.BufferDesc.Width;
            ev.screenHeight = desc.BufferDesc.Height;
            ev.fps = fps;

            selaura::get<selaura::event_manager>().dispatch(ev);
            renderer.render_batch(desc.BufferDesc.Width, desc.BufferDesc.Height);

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            selaura::get<console>().render();

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        if (is_d3d12 && d3d11on12_device && wrapped_back_buffer) {
            main_rtv = nullptr;
            auto resource_ptr = wrapped_back_buffer.get();
            d3d11on12_device->ReleaseWrappedResources(&resource_ptr, 1);
            wrapped_back_buffer = nullptr;
            device_ctx->Flush();
        }

        return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);
    }
};

template<>
struct selaura::detour<&IDXGISwapChain::ResizeBuffers> {
    static HRESULT hk(IDXGISwapChain* thisptr, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
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
};

template<>
struct selaura::detour<&ID3D12CommandQueue::ExecuteCommandLists> {
    static void hk(ID3D12CommandQueue* thisptr, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
        if (!d3d12_queue) d3d12_queue = thisptr;
        return selaura::hook<&ID3D12CommandQueue::ExecuteCommandLists>::call(thisptr, NumCommandLists, ppCommandLists);
    }
};

template<>
struct selaura::detour<&bgfx::d3d11::RendererContextD3D11::submit> {
    static void hk(bgfx::d3d11::RendererContextD3D11* thisptr, void* a1, void* a2, void* a3) {
        selaura::hook<&bgfx::d3d11::RendererContextD3D11::submit>::call(thisptr, a1, a2, a3);

        if (!hooks_init) {
            selaura::hook<&IDXGISwapChain::Present>::enable(thisptr->$getSwapChain());
            selaura::hook<&IDXGISwapChain::ResizeBuffers>::enable(thisptr->$getSwapChain());
            hooks_init = true;
        }
    }
};

template<>
struct selaura::detour<&bgfx::d3d12::RendererContextD3D12::submit> {
    static void hk(bgfx::d3d12::RendererContextD3D12* thisptr, void* a1, void* a2, void* a3) {
        selaura::hook<&bgfx::d3d12::RendererContextD3D12::submit>::call(thisptr, a1, a2, a3);

        if (!hooks_init) {
            auto swapChain = thisptr->$getSwapChain();

            winrt::com_ptr<ID3D12Device> temp_device;
            swapChain->GetDevice(IID_PPV_ARGS(temp_device.put()));

            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 1;

            winrt::com_ptr<ID3D12CommandQueue> dummy_queue;
            temp_device->CreateCommandQueue(&desc, IID_PPV_ARGS(dummy_queue.put()));

            selaura::hook<&ID3D12CommandQueue::ExecuteCommandLists>::enable(dummy_queue.get());
            selaura::hook<&IDXGISwapChain::Present>::enable(swapChain);
            selaura::hook<&IDXGISwapChain::ResizeBuffers>::enable(swapChain);

            hooks_init = true;
        }
    }
};