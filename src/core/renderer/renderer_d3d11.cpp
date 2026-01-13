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

        D3D11_RASTERIZER_DESC r_desc{};
        r_desc.FillMode = D3D11_FILL_SOLID;
        r_desc.CullMode = D3D11_CULL_NONE;
        r_desc.ScissorEnable = FALSE;
        device->CreateRasterizerState(&r_desc, raster_scissor_off.put());

        r_desc.ScissorEnable = TRUE;
        device->CreateRasterizerState(&r_desc, raster_scissor_on.put());

        D3D11_SAMPLER_DESC s_desc{};
        s_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        s_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.MipLODBias = -0.75f;
        s_desc.MaxLOD = D3D11_FLOAT32_MAX;
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
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        ds_desc.StencilEnable = FALSE;
        device->CreateDepthStencilState(&ds_desc, depth_disabled_state.put());

        D3D11_BUFFER_DESC cb_desc = { sizeof(DirectX::XMMATRIX), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE };
        device->CreateBuffer(&cb_desc, nullptr, constant_buffer.put());

        create_shader();

        return true;
    }

    void renderer_d3d11::render(const draw_data& data) {
        if (data.vertices.empty()) return;

        if (!v_buffer || v_capacity < data.vertices.size()) {
            v_capacity = data.vertices.size();
            if (v_capacity < 1024) v_capacity = 1024;
            v_capacity *= 2;

            D3D11_BUFFER_DESC v_desc = {
                static_cast<UINT>(v_capacity * sizeof(vertex)),
                D3D11_USAGE_DYNAMIC,
                D3D11_BIND_VERTEX_BUFFER,
                D3D11_CPU_ACCESS_WRITE
            };
            device->CreateBuffer(&v_desc, nullptr, v_buffer.put());
        }

        size_t needed_indices = (data.vertices.size() / 4) * 6;
        if (!i_buffer || i_capacity < needed_indices) {
            i_capacity = needed_indices;
            if (i_capacity < 1536) i_capacity = 1536;
            i_capacity *= 2;

            D3D11_BUFFER_DESC i_desc = {
                static_cast<UINT>(i_capacity * sizeof(uint32_t)),
                D3D11_USAGE_DYNAMIC,
                D3D11_BIND_INDEX_BUFFER,
                D3D11_CPU_ACCESS_WRITE
            };
            device->CreateBuffer(&i_desc, nullptr, i_buffer.put());
        }

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(ctx->Map(v_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            memcpy(mapped.pData, data.vertices.data(), data.vertices.size() * sizeof(vertex));
            ctx->Unmap(v_buffer.get(), 0);
        }

        if (SUCCEEDED(ctx->Map(i_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            uint32_t* indices = static_cast<uint32_t*>(mapped.pData);
            for (uint32_t i = 0, v = 0; i < needed_indices; i += 6, v += 4) {
                indices[i + 0] = v + 0; indices[i + 1] = v + 1; indices[i + 2] = v + 2;
                indices[i + 3] = v + 0; indices[i + 4] = v + 2; indices[i + 5] = v + 3;
            }
            ctx->Unmap(i_buffer.get(), 0);
        }

        if (SUCCEEDED(ctx->Map(constant_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(0, data.display_size.x, data.display_size.y, 0, 0, 1);
            *static_cast<DirectX::XMMATRIX*>(mapped.pData) = DirectX::XMMatrixTranspose(projection);
            ctx->Unmap(constant_buffer.get(), 0);
        }

        D3D11_VIEWPORT vp = { 0.0f, 0.0f, data.display_size.x, data.display_size.y, 0.0f, 1.0f };
        ctx->RSSetViewports(1, &vp);

        ID3D11RenderTargetView* rtvs[] = { rtv.get() };
        ctx->OMSetRenderTargets(1, rtvs, nullptr);

        float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
        ctx->OMSetBlendState(blend_state.get(), blend_factor, 0xFFFFFFFF);
        ctx->OMSetDepthStencilState(depth_disabled_state.get(), 0);
        ctx->RSSetState(raster_scissor_off.get());

        ID3D11Buffer* vbs[] = { v_buffer.get() };
        UINT strides[] = { sizeof(vertex) }, offsets[] = { 0 };
        ctx->IASetVertexBuffers(0, 1, vbs, strides, offsets);
        ctx->IASetIndexBuffer(i_buffer.get(), DXGI_FORMAT_R32_UINT, 0);
        ctx->IASetInputLayout(layout.get());
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ctx->VSSetShader(vs.get(), nullptr, 0);
        ID3D11Buffer* cbs[] = { constant_buffer.get() };
        ctx->VSSetConstantBuffers(0, 1, cbs);
        ctx->PSSetShader(ps.get(), nullptr, 0);

        ID3D11SamplerState* samplers[] = { sampler.get() };
        ctx->PSSetSamplers(0, 1, samplers);

        uint32_t index_offset = 0;
        uint32_t base_vertex = 0;
        for (const auto& cmd : data.commands) {
            ID3D11ShaderResourceView* srv = static_cast<ID3D11ShaderResourceView*>(cmd.texture);
            ctx->PSSetShaderResources(0, 1, &srv);

            if (cmd.clip_enabled) {
                D3D11_RECT rect = {
                    static_cast<LONG>(cmd.clip_rect.x),
                    static_cast<LONG>(cmd.clip_rect.y),
                    static_cast<LONG>(cmd.clip_rect.x + cmd.clip_rect.z),
                    static_cast<LONG>(cmd.clip_rect.y + cmd.clip_rect.w)
                };
                ctx->RSSetState(raster_scissor_on.get());
                ctx->RSSetScissorRects(1, &rect);
            } else {
                ctx->RSSetState(raster_scissor_off.get());
            }

            uint32_t indices_to_draw = cmd.count;
            ctx->DrawIndexed(indices_to_draw, index_offset, 0);

            index_offset += indices_to_draw;
        }
    }

    void renderer_d3d11::shutdown() {
        if (ctx) {
            ID3D11RenderTargetView* null_rtv = nullptr;
            ctx->OMSetRenderTargets(0, &null_rtv, nullptr);
            ctx->ClearState();
            ctx->Flush();
        }
        rtv = nullptr;
        back_buffer = nullptr;
        v_buffer = nullptr;
        i_buffer = nullptr;
        constant_buffer = nullptr;
    }

    void renderer_d3d11::create_texture(void* data, int width, int height, void** out_srv) {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA sub{};
        sub.pSysMem = data;
        sub.SysMemPitch = width * 4;

        winrt::com_ptr<ID3D11Texture2D> tex;
        device->CreateTexture2D(&desc, &sub, tex.put());
        device->CreateShaderResourceView(tex.get(), nullptr, reinterpret_cast<ID3D11ShaderResourceView**>(out_srv));
    }

    void renderer_d3d11::destroy_texture(texture_id handle) {
        if (handle) {
            auto srv = static_cast<ID3D11ShaderResourceView*>(handle);
            srv->Release();
        }
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
}