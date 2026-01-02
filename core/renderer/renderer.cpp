#include "renderer.hpp"
#include "shaders.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
        if (c.r > 1.0f || c.g > 1.0f || c.b > 1.0f || c.a > 1.0f)
            return { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
        return c;
    }

    ID3D11ShaderResourceView* renderer_impl::get_or_create_texture(Resource res) {
        if (texture_cache.count(res.begin())) return texture_cache[res.begin()].get();

        int w, h, n;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load_from_memory((const unsigned char*)res.data(), (int)res.size(), &w, &h, &n, 4);
        if (!data) return nullptr;

        D3D11_TEXTURE2D_DESC desc{ (UINT)w, (UINT)h, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, {1, 0}, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE };
        D3D11_SUBRESOURCE_DATA sub{ data, (UINT)(w * 4) };

        winrt::com_ptr<ID3D11Texture2D> tex;
        device->CreateTexture2D(&desc, &sub, tex.put());
        winrt::com_ptr<ID3D11ShaderResourceView> srv;
        device->CreateShaderResourceView(tex.get(), nullptr, srv.put());

        stbi_image_free(data);
        texture_cache[res.begin()] = srv;
        return srv.get();
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
        float cx = x + w * 0.5f, cy = y + h * 0.5f;
        struct { float x, y; } corners[4] = { {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h} };
        glm::vec4 c[4];
        glm::vec4 data = { w, h, radius, type };

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

        if (!tess.commands.empty() &&
            tess.commands.back().blend == tess.current_blend &&
            tess.commands.back().texture == tess.current_texture) {
            tess.commands.back().count += 6;
        } else {
            tess.commands.push_back({ 6, tess.current_blend, tess.current_texture });
        }
    }

    void renderer_impl::draw_filled_rect(float x, float y, float w, float h, float radius, glm::vec4 color, float rotation) {
        tess.current_texture = nullptr;
        glm::vec4 colors[4] = { color, color, color, color };
        push_rect_gradient(x, y, w, h, radius, colors, (float)draw_type::rect, rotation);
    }

    void renderer_impl::draw_gradient_rect(float x, float y, float w, float h, float radius, glm::vec4 col_tl, glm::vec4 col_tr, glm::vec4 col_bl, glm::vec4 col_br, float rotation) {
        tess.current_texture = nullptr;
        glm::vec4 colors[4] = { col_tl, col_tr, col_br, col_bl };
        push_rect_gradient(x, y, w, h, radius, colors, (float)draw_type::rect, rotation);
    }

    void renderer_impl::draw_circle(float x, float y, float radius, glm::vec4 color, float rotation) {
        tess.current_texture = nullptr;
        glm::vec4 colors[4] = { color, color, color, color };
        push_rect_gradient(x - radius, y - radius, radius * 2, radius * 2, radius, colors, (float)draw_type::circle, rotation);
    }

    void renderer_impl::draw_image(Resource res, float x, float y, float w, float h, float radius, glm::vec4 tint, float rotation) {
        tess.current_texture = get_or_create_texture(res);
        glm::vec4 colors[4] = { tint, tint, tint, tint };
        push_rect_gradient(x, y, w, h, radius, colors, (float)draw_type::image, rotation);
    }

    gif_data* renderer_impl::get_or_create_gif(Resource res) {
        if (gif_cache.count(res.begin())) return &gif_cache[res.begin()];

        int w, h, frames, channels;
        int* delays = nullptr;
        unsigned char* data = stbi_load_gif_from_memory(
            (const unsigned char*)res.data(), (int)res.size(),
            &delays, &w, &h, &frames, &channels, 4
        );

        if (!data) return nullptr;

        D3D11_TEXTURE2D_DESC desc{ (UINT)w, (UINT)h, 1, (UINT)frames,
                                    DXGI_FORMAT_R8G8B8A8_UNORM, {1, 0},
                                    D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE };

        std::vector<D3D11_SUBRESOURCE_DATA> subdata(frames);
        for (int i = 0; i < frames; i++) {
            subdata[i].pSysMem = data + (i * w * h * 4);
            subdata[i].SysMemPitch = w * 4;
        }

        winrt::com_ptr<ID3D11Texture2D> tex_array;
        device->CreateTexture2D(&desc, subdata.data(), tex_array.put());

        gif_data new_gif;
        device->CreateShaderResourceView(tex_array.get(), nullptr, new_gif.srv.put());
        new_gif.frame_count = frames;
        new_gif.total_duration = 0;
        for (int i = 0; i < frames; i++) {
            new_gif.delays.push_back(delays[i]);
            new_gif.total_duration += delays[i];
        }

        stbi_image_free(data);
        stbi_image_free(delays);

        gif_cache[res.begin()] = std::move(new_gif);
        return &gif_cache[res.begin()];
    }

    void renderer_impl::draw_gif(Resource res, float x, float y, float w, float h, float radius) {
        auto gif = get_or_create_gif(res);
        if (!gif) return;

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        int time_in_loop = now % gif->total_duration;
        int current_frame = 0;
        int accum_time = 0;
        for (int i = 0; i < gif->frame_count; i++) {
            accum_time += gif->delays[i];
            if (time_in_loop < accum_time) {
                current_frame = i;
                break;
            }
        }

        tess.current_texture = gif->srv.get();
        glm::vec4 white[4] = { {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1} };

        push_rect_gradient(x, y, w, h, radius, white, 2.0f + (float)current_frame, 0.0f);
    }

    void renderer_impl::set_font(Resource tex_res, Resource json_res) {
        if (font_cache.count(tex_res.begin())) {
            current_font = &font_cache[tex_res.begin()];
            return;
        }

        font_data font;
        int w, h, n;

        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load_from_memory(
            (const unsigned char*)tex_res.data(),
            (int)tex_res.size(), &w, &h, &n, 4
        );

        if (!data) return;

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = (UINT)w;
        desc.Height = (UINT)h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA sub{};
        sub.pSysMem = data;
        sub.SysMemPitch = (UINT)(w * 4);

        winrt::com_ptr<ID3D11Texture2D> tex;
        device->CreateTexture2D(&desc, &sub, tex.put());

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = desc.Format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srv_desc.Texture2DArray.ArraySize = 1;
        srv_desc.Texture2DArray.FirstArraySlice = 0;
        srv_desc.Texture2DArray.MipLevels = 1;

        device->CreateShaderResourceView(tex.get(), &srv_desc, font.texture.put());
        stbi_image_free(data);

        auto j = nlohmann::json::parse(std::string_view((char*)json_res.data(), json_res.size()));

        float atlas_w = j["atlas"]["width"];
        float atlas_h = j["atlas"]["height"];
        font.px_range = j["atlas"]["distanceRange"];

        for (auto& g : j["glyphs"]) {
            uint32_t unicode = g["unicode"];
            glyph& gl = font.glyphs[unicode];

            gl.advance = g.value("advance", 0.0f);

            if (g.contains("planeBounds")) {
                auto& b = g["planeBounds"];
                gl.x0 = b["left"];
                gl.y0 = b["bottom"];
                gl.x1 = b["right"];
                gl.y1 = b["top"];
            } else {
                gl.x0 = gl.y0 = gl.x1 = gl.y1 = 0.0f;
            }

            if (g.contains("atlasBounds")) {
                auto& b = g["atlasBounds"];
                float tw = j["atlas"]["width"];
                float th = j["atlas"]["height"];

                gl.u0 = (float)b["left"] / tw;
                gl.u1 = (float)b["right"] / tw;

                gl.v0 = 1.0f - ((float)b["top"] / th);
                gl.v1 = 1.0f - ((float)b["bottom"] / th);
            }
        }

        font_cache[tex_res.begin()] = std::move(font);
        current_font = &font_cache[tex_res.begin()];
    }

    void renderer_impl::draw_text(std::string_view text, float x, float y, float size, glm::vec4 color) {
        if (!current_font) return;

        tess.current_texture = current_font->texture.get();
        float cursor_x = x;

        for (uint32_t c : text) {
            if (!current_font->glyphs.count(c)) continue;
            const auto& g = current_font->glyphs.at(c);

            float gw = (g.x1 - g.x0) * size;
            float gh = (g.y1 - g.y0) * size;
            float gx = cursor_x + g.x0 * size;
            float gy = y - g.y1 * size;

            glm::vec4 colors[4] = { color, color, color, color };
            push_rect_gradient(gx, gy, gw, gh, 0.0f, colors, 3.0f, 0.0f);

            auto& v = tess.vertices;
            size_t idx = v.size() - 6;

            v[idx+0].uv = {g.u0, g.v0};
            v[idx+1].uv = {g.u1, g.v0};
            v[idx+2].uv = {g.u0, g.v1};
            v[idx+3].uv = {g.u1, g.v0};
            v[idx+4].uv = {g.u1, g.v1};
            v[idx+5].uv = {g.u0, g.v1};

            cursor_x += g.advance * size;
        }
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
        mult_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        mult_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        device->CreateBlendState(&mult_desc, blend_multiply.put());

        D3D11_DEPTH_STENCIL_DESC ds_desc{ FALSE, D3D11_DEPTH_WRITE_MASK_ZERO };
        device->CreateDepthStencilState(&ds_desc, depth_disabled.put());
        D3D11_RASTERIZER_DESC r_desc{ D3D11_FILL_SOLID, D3D11_CULL_NONE };
        device->CreateRasterizerState(&r_desc, raster_state.put());
        D3D11_SAMPLER_DESC s_desc{};
        s_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        s_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        s_desc.MinLOD = 0;
        s_desc.MaxLOD = D3D11_FLOAT32_MAX;
        s_desc.MipLODBias = -0.5f;

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
            ctx->PSSetShaderResources(0, 1, &cmd.texture);
            ctx->PSSetShader(ps.get(), nullptr, 0);
            ctx->Draw(cmd.count, current_vertex);
            current_vertex += cmd.count;
        }
        tess.clear();
    }
}