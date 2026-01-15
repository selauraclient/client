#include "renderer_d3d11.hpp"
#include <shaders_vs.h>
#include <shaders_ps.h>

namespace sgfx {
    bool renderer_d3d11::init(void* sc) {
        swapchain.copy_from(static_cast<IDXGISwapChain3*>(sc));
        if (FAILED(swapchain->GetDevice(IID_PPV_ARGS(device.put())))) return false;
        device->GetImmediateContext(ctx.put());

        winrt::com_ptr<ID3D11Texture2D> bb;
        swapchain->GetBuffer(0, IID_PPV_ARGS(bb.put()));
        device->CreateRenderTargetView(bb.get(), nullptr, rtv.put());

        D3D11_TEXTURE2D_DESC bb_desc;
        bb->GetDesc(&bb_desc);
        screen_size = { (float)bb_desc.Width, (float)bb_desc.Height };
        create_blur_resources(bb_desc);

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
        return true;
    }

    void renderer_d3d11::create_blur_resources(const D3D11_TEXTURE2D_DESC& bb_desc) {
        D3D11_TEXTURE2D_DESC desc = bb_desc;
        desc.SampleDesc.Count = 1;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.Usage = D3D11_USAGE_DEFAULT;

        for (int i = 0; i < 2; ++i) {
            blur_textures[i] = nullptr; blur_srvs[i] = nullptr; blur_rtvs[i] = nullptr;
            device->CreateTexture2D(&desc, nullptr, blur_textures[i].put());
            device->CreateShaderResourceView(blur_textures[i].get(), nullptr, blur_srvs[i].put());
            device->CreateRenderTargetView(blur_textures[i].get(), nullptr, blur_rtvs[i].put());
        }
    }

    void renderer_d3d11::render(const draw_data& data) {
        if (data.vertices.empty()) return;

        if (data.display_size.x != screen_size.x || data.display_size.y != screen_size.y) {
            winrt::com_ptr<ID3D11Texture2D> bb;
            swapchain->GetBuffer(0, IID_PPV_ARGS(bb.put()));
            D3D11_TEXTURE2D_DESC bb_desc;
            bb->GetDesc(&bb_desc);
            screen_size = data.display_size;
            create_blur_resources(bb_desc);
        }

        if (!v_buffer || v_capacity < data.vertices.size()) {
            v_capacity = data.vertices.size() * 1.5;
            D3D11_BUFFER_DESC v_desc = { (UINT)(v_capacity * sizeof(vertex)), D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE };
            device->CreateBuffer(&v_desc, nullptr, v_buffer.put());
        }

        size_t needed_indices = (data.vertices.size() / 4) * 6;
        if (!i_buffer || i_capacity < needed_indices) {
            i_capacity = needed_indices * 1.5;
            D3D11_BUFFER_DESC i_desc = { (UINT)(i_capacity * sizeof(uint32_t)), D3D11_USAGE_DYNAMIC, D3D11_BIND_INDEX_BUFFER, D3D11_CPU_ACCESS_WRITE };
            device->CreateBuffer(&i_desc, nullptr, i_buffer.put());
        }

        D3D11_MAPPED_SUBRESOURCE m;
        if (SUCCEEDED(ctx->Map(v_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) {
            memcpy(m.pData, data.vertices.data(), data.vertices.size() * sizeof(vertex));
            ctx->Unmap(v_buffer.get(), 0);
        }

        if (SUCCEEDED(ctx->Map(i_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) {
            uint32_t* idx = (uint32_t*)m.pData;
            for (uint32_t i = 0, v = 0; i < (uint32_t)needed_indices; i += 6, v += 4) {
                idx[i+0]=v+0; idx[i+1]=v+1; idx[i+2]=v+2; idx[i+3]=v+0; idx[i+4]=v+2; idx[i+5]=v+3;
            }
            ctx->Unmap(i_buffer.get(), 0);
        }

        if (SUCCEEDED(ctx->Map(constant_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) {
            DirectX::XMMATRIX p = DirectX::XMMatrixOrthographicOffCenterLH(0, data.display_size.x, data.display_size.y, 0, 0, 1);
            *static_cast<DirectX::XMMATRIX*>(m.pData) = DirectX::XMMatrixTranspose(p);
            ctx->Unmap(constant_buffer.get(), 0);
        }

        D3D11_VIEWPORT vp = { 0, 0, data.display_size.x, data.display_size.y, 0, 1 };
        ctx->RSSetViewports(1, &vp);
        ID3D11RenderTargetView* main_rtv = rtv.get();
        ctx->OMSetRenderTargets(1, &main_rtv, nullptr);

        float bf[4] = {0,0,0,0};
        ctx->OMSetBlendState(blend_state.get(), bf, 0xffffffff);
        ctx->OMSetDepthStencilState(depth_disabled_state.get(), 0);
        ctx->IASetInputLayout(layout.get());
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D11Buffer* vbs[] = { v_buffer.get() };
        UINT str[] = { sizeof(vertex) }, off[] = { 0 };
        ctx->IASetVertexBuffers(0, 1, vbs, str, off);
        ctx->IASetIndexBuffer(i_buffer.get(), DXGI_FORMAT_R32_UINT, 0);

        ctx->VSSetShader(vs.get(), nullptr, 0);
        ID3D11Buffer* cbs[] = { constant_buffer.get() };
        ctx->VSSetConstantBuffers(0, 1, cbs);
        ctx->PSSetShader(ps.get(), nullptr, 0);

        ID3D11SamplerState* samplers[] = { sampler.get() };
        ctx->PSSetSamplers(0, 1, samplers);

        uint32_t idx_off = 0;
        uint32_t v_off = 0;

        for (const auto& cmd : data.commands) {
            if (cmd.is_blur) {
                winrt::com_ptr<ID3D11Resource> back_res;
                main_rtv->GetResource(back_res.put());
                D3D11_TEXTURE2D_DESC bd;
                static_cast<ID3D11Texture2D*>(back_res.get())->GetDesc(&bd);

                if (bd.SampleDesc.Count > 1) {
                    ctx->ResolveSubresource(blur_textures[0].get(), 0, back_res.get(), 0, bd.Format);
                } else {
                    ctx->CopyResource(blur_textures[0].get(), back_res.get());
                }

                int last_dest = 0;
                for (int i = 0; i < cmd.blur_iterations; ++i) {
                    int s = i % 2;
                    int d = (i + 1) % 2;
                    last_dest = d;

                    ID3D11RenderTargetView* t_rtv = blur_rtvs[d].get();
                    ctx->OMSetRenderTargets(1, &t_rtv, nullptr);

                    ID3D11ShaderResourceView* s_srv = blur_srvs[s].get();
                    ctx->PSSetShaderResources(0, 1, &s_srv);

                    D3D11_MAPPED_SUBRESOURCE bm;
                    if (SUCCEEDED(ctx->Map(blur_cb.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bm))) {
                        blur_params* p = (blur_params*)bm.pData;
                        p->offset = (float)i * cmd.blur_intensity;
                        ctx->Unmap(blur_cb.get(), 0);
                    }
                    ID3D11Buffer* pbcb = blur_cb.get();
                    ctx->PSSetConstantBuffers(1, 1, &pbcb);

                    ctx->DrawIndexed(6, idx_off, 0);

                    ID3D11ShaderResourceView* null_srv = nullptr;
                    ctx->PSSetShaderResources(0, 1, &null_srv);
                }

                ctx->OMSetRenderTargets(1, &main_rtv, nullptr);

                ID3D11ShaderResourceView* final_srv = blur_srvs[last_dest].get();
                ctx->PSSetShaderResources(0, 1, &final_srv);

                ctx->DrawIndexed(6, idx_off, 0);
            } else {
                ID3D11ShaderResourceView* srv = static_cast<ID3D11ShaderResourceView*>(cmd.texture);
                ctx->PSSetShaderResources(0, 1, &srv);

                if (cmd.clip_enabled) {
                    D3D11_RECT r = {(LONG)cmd.clip_rect.x, (LONG)cmd.clip_rect.y, (LONG)(cmd.clip_rect.x+cmd.clip_rect.z), (LONG)(cmd.clip_rect.y+cmd.clip_rect.w)};
                    ctx->RSSetState(raster_scissor_on.get());
                    ctx->RSSetScissorRects(1, &r);
                } else {
                    ctx->RSSetState(raster_scissor_off.get());
                }

                ctx->DrawIndexed(cmd.count, idx_off, 0);
            }
            idx_off += cmd.count;
            v_off += (cmd.count / 6) * 4 * sizeof(vertex);
        }
    }

    void renderer_d3d11::shutdown() {
        if (ctx) {
            ID3D11RenderTargetView* null_rtv = nullptr;
            ctx->OMSetRenderTargets(0, &null_rtv, nullptr);
            ctx->ClearState();
            ctx->Flush();
        }
        rtv = nullptr; swapchain = nullptr; device = nullptr; ctx = nullptr;
        v_buffer = nullptr; i_buffer = nullptr; constant_buffer = nullptr; blur_cb = nullptr;
        vs = nullptr; ps = nullptr; layout = nullptr; sampler = nullptr;
        blend_state = nullptr; raster_scissor_off = nullptr; raster_scissor_on = nullptr;
        depth_disabled_state = nullptr;
        for(int i=0; i<2; ++i) { blur_textures[i] = nullptr; blur_srvs[i] = nullptr; blur_rtvs[i] = nullptr; }
    }

    void renderer_d3d11::create_texture(void* data, int width, int height, void** out_srv) {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width; desc.Height = height; desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sub{};
        sub.pSysMem = data; sub.SysMemPitch = width * 4;
        winrt::com_ptr<ID3D11Texture2D> tex;
        device->CreateTexture2D(&desc, &sub, tex.put());
        device->CreateShaderResourceView(tex.get(), nullptr, reinterpret_cast<ID3D11ShaderResourceView**>(out_srv));
    }

    void renderer_d3d11::destroy_texture(texture_id handle) {
        if (handle) static_cast<IUnknown*>(handle)->Release();
    }

    void renderer_d3d11::create_shader() {
        device->CreateVertexShader(g_shaders_vs, sizeof(g_shaders_vs), nullptr, vs.put());
        device->CreatePixelShader(g_shaders_ps, sizeof(g_shaders_ps), nullptr, ps.put());
        D3D11_INPUT_ELEMENT_DESC layout_desc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(vertex, pos),   D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(vertex, col),   D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(vertex, uv),    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"DATA",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(vertex, data),  D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"RADII",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(vertex, radii), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        device->CreateInputLayout(layout_desc, 5, g_shaders_vs, sizeof(g_shaders_vs), layout.put());
    }

    void* renderer_d3d11::get_device() {
        return this->device.get();
    }
}