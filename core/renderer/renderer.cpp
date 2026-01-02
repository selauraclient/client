#include "renderer.hpp"
#include "shaders.hpp"

namespace selaura {
    void render_buffer::update(ID3D11Device* device, ID3D11DeviceContext* ctx, const std::vector<vertex>& data) {
        if (data.empty()) return;
        if (!buffer || data.size() > capacity) {
            capacity = data.size() + 256;
            D3D11_BUFFER_DESC desc{ (UINT)(sizeof(vertex) * capacity), D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE };
            device->CreateBuffer(&desc, nullptr, buffer.put());
        }
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(ctx->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            memcpy(mapped.pData, data.data(), sizeof(vertex) * data.size());
            ctx->Unmap(buffer.get(), 0);
        }
    }

    glm::vec4 renderer_impl::normalize_color(glm::vec4 c) {
        return (c.r > 1.0f || c.g > 1.0f || c.b > 1.0f) ? glm::vec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f) : c;
    }

    void rotate_point(float& x, float& y, float cx, float cy, float angle) {
        if (angle == 0.0f) return;
        float rad = angle * (3.14159265f / 180.0f);
        float s = sin(rad), c = cos(rad);
        x -= cx; y -= cy;
        float xnew = x * c - y * s;
        float ynew = x * s + y * c;
        x = xnew + cx; y = ynew + cy;
    }

    void renderer_impl::push_rect_gradient(float x, float y, float w, float h, float radius, glm::vec4 colors[4], float type, float rotation) {
        glm::vec4 data = { w, h, radius, type };
        float cx = x + w * 0.5f, cy = y + h * 0.5f;

        struct { float x, y; } corners[4] = { {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h} };
        glm::vec4 c[4];

        for (int i = 0; i < 4; i++) {
            c[i] = normalize_color(colors[i]);
            rotate_point(corners[i].x, corners[i].y, cx, cy, rotation);
        }

        vertex v[6] = {
            {{corners[0].x, corners[0].y, 0.5f}, c[0], {0, 0}, data},
            {{corners[1].x, corners[1].y, 0.5f}, c[1], {1, 0}, data},
            {{corners[3].x, corners[3].y, 0.5f}, c[3], {0, 1}, data},
            {{corners[1].x, corners[1].y, 0.5f}, c[1], {1, 0}, data},
            {{corners[2].x, corners[2].y, 0.5f}, c[2], {1, 1}, data},
            {{corners[3].x, corners[3].y, 0.5f}, c[3], {0, 1}, data}
        };

        for (int i = 0; i < 6; i++) tess.vertices.push_back(v[i]);

        if (!tess.commands.empty() && tess.commands.back().blend == tess.current_blend) {
            tess.commands.back().count += 6;
        } else {
            tess.commands.push_back({ 6, false, tess.current_blend });
        }
    }

    void renderer_impl::draw_filled_rect(float x, float y, float w, float h, float radius, glm::vec4 color, float rotation) {
        glm::vec4 colors[4] = { color, color, color, color };
        push_rect_gradient(x, y, w, h, radius, colors, 0.0f, rotation);
    }

    void renderer_impl::draw_gradient_rect(float x, float y, float w, float h, float radius, glm::vec4 col_tl, glm::vec4 col_tr, glm::vec4 col_bl, glm::vec4 col_br, float rotation) {
        glm::vec4 colors[4] = { col_tl, col_tr, col_br, col_bl };
        push_rect_gradient(x, y, w, h, radius, colors, 0.0f, rotation);
    }

    void renderer_impl::draw_circle(float x, float y, float radius, glm::vec4 color, float rotation) {
        glm::vec4 colors[4] = { color, color, color, color };
        push_rect_gradient(x - radius, y - radius, radius * 2, radius * 2, radius, colors, 1.0f, rotation);
    }

    void renderer_impl::init(ID3D11Device* p_device) {
        device.copy_from(p_device);
        device->GetImmediateContext(ctx.put());

        winrt::com_ptr<ID3DBlob> vs_blob, ps_blob;
        D3DCompile(shaders::geometry::vertex, strlen(shaders::geometry::vertex), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, vs_blob.put(), nullptr);
        D3DCompile(shaders::geometry::pixel, strlen(shaders::geometry::pixel), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, ps_blob.put(), nullptr);

        device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, vs.put());
        device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, ps.put());

        D3D11_INPUT_ELEMENT_DESC desc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "DATA", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        device->CreateInputLayout(desc, 4, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), layout.put());

        D3D11_BUFFER_DESC cb_desc{ sizeof(MatrixBuffer), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE };
        device->CreateBuffer(&cb_desc, nullptr, constant_buffer.put());

        D3D11_BLEND_DESC b_desc{};
        b_desc.RenderTarget[0].BlendEnable = TRUE;
        b_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        b_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        b_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        b_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        b_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        b_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        b_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&b_desc, blend_enabled.put());

        D3D11_BLEND_DESC mult_desc = b_desc;
        mult_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
        mult_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        mult_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

        mult_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        mult_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        mult_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        device->CreateBlendState(&mult_desc, blend_multiply.put());

        D3D11_DEPTH_STENCIL_DESC ds_desc{ FALSE, D3D11_DEPTH_WRITE_MASK_ZERO };
        device->CreateDepthStencilState(&ds_desc, depth_disabled.put());
        D3D11_RASTERIZER_DESC r_desc{ D3D11_FILL_SOLID, D3D11_CULL_NONE };
        device->CreateRasterizerState(&r_desc, raster_state.put());
        D3D11_SAMPLER_DESC s_desc{ D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP };
        device->CreateSamplerState(&s_desc, sampler.put());
    }

    void renderer_impl::render_batch(float screen_w, float screen_h) {
        if (tess.vertices.empty()) return;
        v_buffer.update(device.get(), ctx.get(), tess.vertices);

        D3D11_VIEWPORT vp{ 0, 0, screen_w, screen_h, 0, 1 };
        ctx->RSSetViewports(1, &vp);
        ctx->OMSetDepthStencilState(depth_disabled.get(), 0);
        ctx->RSSetState(raster_state.get());

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(ctx->Map(constant_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            ((MatrixBuffer*)mapped.pData)->projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicOffCenterLH(0, screen_w, screen_h, 0, 0, 1));
            ctx->Unmap(constant_buffer.get(), 0);
        }

        UINT stride = sizeof(vertex), offset = 0;
        ID3D11Buffer* vbs[] = { v_buffer.get() };
        ctx->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
        ctx->IASetInputLayout(layout.get());
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(vs.get(), nullptr, 0);
        auto cb = constant_buffer.get();
        ctx->VSSetConstantBuffers(0, 1, &cb);
        ctx->PSSetSamplers(0, 1, sampler.put());

        uint32_t current_vertex = 0;
        for (const auto& cmd : tess.commands) {
            ID3D11BlendState* state = (cmd.blend == blend_mode::multiply) ? blend_multiply.get() : blend_enabled.get();
            ctx->OMSetBlendState(state, nullptr, 0xFFFFFFFF);
            ctx->PSSetShader(ps.get(), nullptr, 0);
            ctx->Draw(cmd.count, current_vertex);
            current_vertex += cmd.count;
        }
        tess.clear();
    }
}