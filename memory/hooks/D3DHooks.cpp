#include <pch.hpp>

static winrt::com_ptr<ID3D11Device> device;
static winrt::com_ptr<ID3D11DeviceContext> device_ctx;
static winrt::com_ptr<ID3D11Texture2D> back_buffer;
static winrt::com_ptr<ID3D11RenderTargetView> main_rtv;

static WNDPROC wnd_proc = nullptr;
static bool d3d_init = false;

template <>
struct selaura::detour<&IDXGISwapChain::Present> {
    static HRESULT hk(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {
        static bool imgui_init = false;
        if (!d3d_init) {
            if (!imgui_init) {
                thisptr->GetDevice(__uuidof(device), device.put_void());
                device->GetImmediateContext(device_ctx.put());

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

            if (SUCCEEDED(thisptr->GetBuffer(0, __uuidof(ID3D11Texture2D), back_buffer.put_void()))) {
                device->CreateRenderTargetView(back_buffer.get(), NULL, main_rtv.put());
            }

            d3d_init = true;
        }

        if (!main_rtv) return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);

        ID3D11RenderTargetView* rtv_ptr = main_rtv.get();
        device_ctx->OMSetRenderTargets(1, &rtv_ptr, NULL);

        static bool renderer_init = false;
        if (!renderer_init) {
            selaura::renderer = std::make_unique<selaura::renderer_impl>();
            selaura::renderer->init(device.get());
            renderer_init = true;
        }

        DXGI_SWAP_CHAIN_DESC desc;
        thisptr->GetDesc(&desc);

        selaura::renderer->draw_gradient_rect(50, 50, 200, 200, 0,
            {255, 255, 255, 255}, {255, 0, 0, 255},
            {255, 255, 255, 255}, {255, 0, 0, 255}, 45);

        selaura::renderer->set_blend_mode(selaura::blend_mode::multiply);
        selaura::renderer->draw_gradient_rect(50, 50, 200, 200, 0,
            {255, 255, 255, 255}, {255, 255, 255, 255},
            {0, 0, 0, 255}, {0, 0, 0, 255},
            45);

        selaura::renderer->set_blend_mode(selaura::blend_mode::normal);
        selaura::renderer->render_batch(desc.BufferDesc.Width, desc.BufferDesc.Height);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        selaura::console->render();

        ImGui::Render();

        selaura::render_event ev{};
        ev.swapChain = thisptr;
        selaura::event_manager->dispatch(ev);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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
struct selaura::detour<&bgfx::d3d11::RenderContextD3D11::submit> {
    static void hk(bgfx::d3d11::RenderContextD3D11* thisptr, void* a1, void* a2, void* a3) {
        selaura::hook<&bgfx::d3d11::RenderContextD3D11::submit>::call(thisptr, a1, a2, a3);

        static bool once = false;
        if (!once) {
            selaura::hook<&IDXGISwapChain::Present>::enable(thisptr->$getSwapChain());
            selaura::hook<&IDXGISwapChain::ResizeBuffers>::enable(thisptr->$getSwapChain());
            once = true;
        }
    }
};