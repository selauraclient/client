#include <pch.hpp>

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include <core/service_manager.hpp>
#include <memory/detour.hpp>
#include <memory/hook.hpp>

#include <sdk/renderer/bgfx.hpp>


#include <core/renderer/sgfx.hpp>
#include <core/renderer/renderer_d3d11.hpp>

static bool hooks_init = false;

LOAD_RESOURCE(Poppins_msdf_png)
LOAD_RESOURCE(Poppins_msdf_json)

LOAD_RESOURCE(Minecraft_msdf_png)
LOAD_RESOURCE(Minecraft_msdf_json)

LOAD_RESOURCE(FontAwesome_msdf_png)
LOAD_RESOURCE(FontAwesome_msdf_json)

LOAD_RESOURCE(selaura_icon_png)

static bool init = false;

template <>
struct selaura::detour<&IDXGISwapChain::Present> {
    static HRESULT hk(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {
        if (!init && sgfx::init(thisptr)) {
            init = true;
        }

        sgfx::begin_frame(sgfx::get_context().backend->get_screen_size());
        sgfx::set_font(GET_RESOURCE(Poppins_msdf_png), GET_RESOURCE(Poppins_msdf_json), false);
        sgfx::draw_text("Hello!", 100, 400, 72);
        sgfx::draw_rect(100, 100, 200, 200, {0, 0.5f, 1.0f, 0.8f}, {15, 0, 0, 15});
        sgfx::draw_image(GET_RESOURCE(selaura_icon_png), 100, 100, 200, 200);
        sgfx::end_frame();

        return selaura::hook<&IDXGISwapChain::Present>::call(thisptr, SyncInterval, Flags);
    }
};

template<>
struct selaura::detour<&IDXGISwapChain::ResizeBuffers> {
    static HRESULT hk(IDXGISwapChain* thisptr, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        sgfx::invalidate();
        init = false;
        return selaura::hook<&IDXGISwapChain::ResizeBuffers>::call(thisptr, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }
};

template<>
struct selaura::detour<&ID3D12CommandQueue::ExecuteCommandLists> {
    static void hk(ID3D12CommandQueue* thisptr, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
        //if (!d3d12_queue) d3d12_queue = thisptr;
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