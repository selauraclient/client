#include "renderer_d3d12.hpp"
#include <string>

namespace sgfx {
    bool renderer_d3d12::init(void* sc) {
        if (!sc) return false;
        if (!d3d12_queue) return false;

        swapchain.copy_from(static_cast<IDXGISwapChain3*>(sc));

        if (FAILED(swapchain->GetDevice(IID_PPV_ARGS(d12_device.put())))) {
            return false;
        }

        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED;
        ID3D12CommandQueue* queue_ptr = d3d12_queue.get();

        if (FAILED(D3D11On12CreateDevice(
            d12_device.get(),
            flags,
            nullptr,
            0,
            reinterpret_cast<IUnknown**>(&queue_ptr),
            1,
            0,
            device.put(),
            ctx.put(),
            nullptr
        ))) return false;

        if (FAILED(device->QueryInterface(IID_PPV_ARGS(d11on12_device.put())))) return false;

        DXGI_SWAP_CHAIN_DESC1 sc_desc;
        swapchain->GetDesc1(&sc_desc);
        buffer_count = sc_desc.BufferCount;

        wrapped_back_buffers.resize(buffer_count);
        wrapped_rtvs.resize(buffer_count);

        for (uint32_t i = 0; i < buffer_count; i++) {
            winrt::com_ptr<ID3D12Resource> res;
            if (FAILED(swapchain->GetBuffer(i, IID_PPV_ARGS(res.put())))) return false;

            D3D11_RESOURCE_FLAGS d11_flags = { D3D11_BIND_RENDER_TARGET };
            if (FAILED(d11on12_device->CreateWrappedResource(
                res.get(),
                &d11_flags,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT,
                IID_PPV_ARGS(wrapped_back_buffers[i].put())
            ))) return false;

            if (FAILED(device->CreateRenderTargetView(wrapped_back_buffers[i].get(), nullptr, wrapped_rtvs[i].put())))
                return false;
        }

        D3D11_RASTERIZER_DESC r_desc{};
        r_desc.FillMode = D3D11_FILL_SOLID;
        r_desc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState(&r_desc, raster_scissor_off.put());
        r_desc.ScissorEnable = TRUE;
        device->CreateRasterizerState(&r_desc, raster_scissor_on.put());

        D3D11_SAMPLER_DESC s_desc{};
        s_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        s_desc.AddressU = s_desc.AddressV = s_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState(&s_desc, sampler.put());

        D3D11_BLEND_DESC b_desc{};
        b_desc.RenderTarget[0].BlendEnable = TRUE;
        b_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        b_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        b_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        b_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        b_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        b_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        b_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&b_desc, blend_state.put());

        D3D11_DEPTH_STENCIL_DESC ds_desc{};
        ds_desc.DepthEnable = FALSE;
        device->CreateDepthStencilState(&ds_desc, depth_disabled_state.put());

        D3D11_BUFFER_DESC cb_desc = { sizeof(DirectX::XMMATRIX), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE };
        device->CreateBuffer(&cb_desc, nullptr, constant_buffer.put());

        D3D11_BUFFER_DESC bcb_desc = { sizeof(blur_params), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE };
        device->CreateBuffer(&bcb_desc, nullptr, blur_cb.put());

        create_shader();

        D3D11_TEXTURE2D_DESC blur_res_desc{};
        blur_res_desc.Width = sc_desc.Width;
        blur_res_desc.Height = sc_desc.Height;
        blur_res_desc.MipLevels = 1;
        blur_res_desc.ArraySize = 1;
        blur_res_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        blur_res_desc.SampleDesc.Count = 1;
        blur_res_desc.Usage = D3D11_USAGE_DEFAULT;
        blur_res_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        create_blur_resources(blur_res_desc);

        screen_size = { (float)sc_desc.Width, (float)sc_desc.Height };
        return true;
    }

    void renderer_d3d12::render(const draw_data& data) {
        if (!d11on12_device || !ctx) return;
        uint32_t back_buffer_idx = swapchain->GetCurrentBackBufferIndex();
        if (back_buffer_idx >= wrapped_back_buffers.size()) return;
        ID3D11Resource* current_wrapped_res = wrapped_back_buffers[back_buffer_idx].get();
        d11on12_device->AcquireWrappedResources(&current_wrapped_res, 1);
        this->rtv = wrapped_rtvs[back_buffer_idx];
        renderer_d3d11::render(data);
        d11on12_device->ReleaseWrappedResources(&current_wrapped_res, 1);
        ctx->Flush();
    }

    void renderer_d3d12::shutdown() {
        if (ctx) ctx->ClearState();
        wrapped_rtvs.clear();
        wrapped_back_buffers.clear();
        d11on12_device = nullptr;
        d12_device = nullptr;
        d3d12_queue = nullptr;
        renderer_d3d11::shutdown();
    }
}