#include "renderer.hpp"
#include "shaders.hpp"

#if defined(__clang__) && defined(_MSC_VER)
    #ifdef __cpuid
        #undef __cpuid
    #endif
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace selaura {
    void rotate_point(float& x, float& y, float cx, float cy, float angle) {
        if (std::abs(angle) < 0.001f) return;

        const float rad = glm::radians(angle);
        const float s = std::sin(rad);
        const float c = std::cos(rad);

        x -= cx;
        y -= cy;

        const float nx = x * c - y * s;
        const float ny = x * s + y * c;

        x = nx + cx;
        y = ny + cy;
    }

    uint32_t decode_utf8(std::string_view text, size_t& i) {
        uint32_t c = (unsigned char)text[i++];
        if (c >= 0x80) {
            if ((c & 0xE0) == 0xC0 && i < text.length()) {
                c = ((c & 0x1F) << 6) | ((unsigned char)text[i++] & 0x3F);
            } else if ((c & 0xF0) == 0xE0 && i + 1 < text.length()) {
                c = ((c & 0x0F) << 12) | (((unsigned char)text[i++] & 0x3F) << 6);
                c |= ((unsigned char)text[i++] & 0x3F);
            } else if ((c & 0xF8) == 0xF0 && i + 2 < text.length()) {
                c = ((c & 0x07) << 18) | (((unsigned char)text[i++] & 0x3F) << 12);
                c |= (((unsigned char)text[i++] & 0x3F) << 6);
                c |= ((unsigned char)text[i++] & 0x3F);
            }
        }
        return c;
    }

    void render_buffer::update(ID3D11Device* device, ID3D11DeviceContext* ctx, const std::vector<vertex>& data) {
        if (data.empty()) return;

        if (!buffer || data.size() > capacity) {
            capacity = data.size() + 256;

            D3D11_BUFFER_DESC desc{};
            desc.ByteWidth      = (UINT)(sizeof(vertex) * capacity);
            desc.Usage          = D3D11_USAGE_DYNAMIC;
            desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            device->CreateBuffer(&desc, nullptr, buffer.put());
        }

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(ctx->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            memcpy(mapped.pData, data.data(), sizeof(vertex) * data.size());
            ctx->Unmap(buffer.get(), 0);
        }
    }

    glm::vec4 renderer::normalize_color(glm::vec4 c) {
        bool is_255 = (c.r > 1.0f || c.g > 1.0f || c.b > 1.0f || c.a > 1.0f);
        return is_255 ? (c / 255.0f) : c;
    }

    ID3D11ShaderResourceView* renderer::get_or_create_texture(Resource res) {
        auto it = texture_cache.find((const char*)res.begin());
        if (it != texture_cache.end()) {
            return it->second.get();
        }

        int w, h, n;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* pixels = stbi_load_from_memory(
            (const unsigned char*)res.data(),
            (int)res.size(), &w, &h, &n, 4
        );

        if (!pixels) {
            spdlog::error("Failed to load texture from memory. Resource size: {}", res.size());
            return nullptr;
        }

        D3D11_TEXTURE2D_DESC tex_desc{};
        tex_desc.Width            = (UINT)w;
        tex_desc.Height           = (UINT)h;
        tex_desc.MipLevels        = 1;
        tex_desc.ArraySize        = 1;
        tex_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        tex_desc.SampleDesc.Count = 1;
        tex_desc.Usage            = D3D11_USAGE_IMMUTABLE;
        tex_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init_data{ pixels, (UINT)(w * 4) };

        winrt::com_ptr<ID3D11Texture2D> tex;
        HRESULT hr = device->CreateTexture2D(&tex_desc, &init_data, tex.put());
        if (FAILED(hr)) {
            spdlog::error("CreateTexture2D failed with HRESULT: 0x{:X}", (unsigned int)hr);
            stbi_image_free(pixels);
            return nullptr;
        }

        winrt::com_ptr<ID3D11ShaderResourceView> srv;
        hr = device->CreateShaderResourceView(tex.get(), nullptr, srv.put());
        if (FAILED(hr)) {
            spdlog::error("CreateShaderResourceView failed with HRESULT: 0x{:X}", (unsigned int)hr);
            stbi_image_free(pixels);
            return nullptr;
        }

        stbi_image_free(pixels);

        return (texture_cache[res.begin()] = srv).get();
    }

    gif_data* renderer::get_or_create_gif(Resource res) {
        if (gif_cache.count(res.begin())) {
            return &gif_cache[res.begin()];
        }

        int w, h, frames, channels;
        int* delays = nullptr;
        unsigned char* data = stbi_load_gif_from_memory(
            (const unsigned char*)res.data(),
            (int)res.size(), &delays, &w, &h, &frames, &channels, 4
        );

        if (!data) return nullptr;

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width            = (UINT)w;
        desc.Height           = (UINT)h;
        desc.MipLevels        = 1;
        desc.ArraySize        = (UINT)frames;
        desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage            = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

        std::vector<D3D11_SUBRESOURCE_DATA> sub_data(frames);
        for (int i = 0; i < frames; i++) {
            sub_data[i].pSysMem     = data + (i * w * h * 4);
            sub_data[i].SysMemPitch = w * 4;
        }

        winrt::com_ptr<ID3D11Texture2D> tex_array;
        device->CreateTexture2D(&desc, sub_data.data(), tex_array.put());

        gif_data new_gif;
        device->CreateShaderResourceView(tex_array.get(), nullptr, new_gif.srv.put());

        new_gif.frame_count    = frames;
        new_gif.total_duration = 0;
        for (int i = 0; i < frames; i++) {
            new_gif.delays.push_back(delays[i]);
            new_gif.total_duration += delays[i];
        }

        stbi_image_free(data);
        stbi_image_free(delays);

        return &(gif_cache[res.begin()] = std::move(new_gif));
    }

    void renderer::push_rect_gradient(float x, float y, float w, float h, float radius, glm::vec4 colors[4], float type, float rotation, float param) {
        float cx = x + w * 0.5f;
        float cy = y + h * 0.5f;

        struct Corner { float x, y; } corners[4] = {
            {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}
        };

        glm::vec4 shader_data = { w, h, (param != -1.0f ? param : radius), type };
        glm::vec4 final_colors[4];

        for (int i = 0; i < 4; i++) {
            final_colors[i] = normalize_color(colors[i]);
            rotate_point(corners[i].x, corners[i].y, cx, cy, rotation);
        }

        vertex v[6] = {
            {{corners[0].x, corners[0].y, 0.5f}, final_colors[0], {0, 0}, shader_data},
            {{corners[1].x, corners[1].y, 0.5f}, final_colors[1], {1, 0}, shader_data},
            {{corners[3].x, corners[3].y, 0.5f}, final_colors[3], {0, 1}, shader_data},
            {{corners[1].x, corners[1].y, 0.5f}, final_colors[1], {1, 0}, shader_data},
            {{corners[2].x, corners[2].y, 0.5f}, final_colors[2], {1, 1}, shader_data},
            {{corners[3].x, corners[3].y, 0.5f}, final_colors[3], {0, 1}, shader_data}
        };

        for (int i = 0; i < 6; i++) {
            tess.vertices.push_back(v[i]);
        }

        bool can_batch = !tess.commands.empty() &&
                         tess.commands.back().blend == tess.current_blend &&
                         tess.commands.back().texture == tess.current_texture;

        if (can_batch) {
            tess.commands.back().count += 6;
        } else {
            tess.commands.push_back({ 6, tess.current_blend, tess.current_texture });
        }
    }

    void renderer::push_rect_ex(float x, float y, float w, float h, glm::vec4 radii, glm::vec4 colors[4], float type, float rotation, float stroke_width) {
        float cx = x + w * 0.5f, cy = y + h * 0.5f;
        struct { float x, y; } corners[4] = { {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h} };

        glm::vec4 shader_data = { w, h, stroke_width, type };
        glm::vec4 final_colors[4];

        for (int i = 0; i < 4; i++) {
            final_colors[i] = normalize_color(colors[i]);
            rotate_point(corners[i].x, corners[i].y, cx, cy, rotation);
        }

        vertex v[6] = {
            {{corners[0].x, corners[0].y, 0.5f}, final_colors[0], {0, 0}, shader_data, radii},
            {{corners[1].x, corners[1].y, 0.5f}, final_colors[1], {1, 0}, shader_data, radii},
            {{corners[3].x, corners[3].y, 0.5f}, final_colors[3], {0, 1}, shader_data, radii},
            {{corners[1].x, corners[1].y, 0.5f}, final_colors[1], {1, 0}, shader_data, radii},
            {{corners[2].x, corners[2].y, 0.5f}, final_colors[2], {1, 1}, shader_data, radii},
            {{corners[3].x, corners[3].y, 0.5f}, final_colors[3], {0, 1}, shader_data, radii}
        };

        for (int i = 0; i < 6; i++) tess.vertices.push_back(v[i]);

        if (tess.commands.empty() || tess.commands.back().texture != tess.current_texture || tess.commands.back().blend != tess.current_blend)
            tess.commands.push_back({ 6, tess.current_blend, tess.current_texture });
        else
            tess.commands.back().count += 6;
    }

    void renderer::draw_filled_rect(float x, float y, float w, float h, glm::vec4 radii, glm::vec4 color, float rotation) {
        tess.current_texture = nullptr;
        glm::vec4 colors[4] = { color, color, color, color };
        push_rect_ex(x, y, w, h, radii, colors, 0.0f, rotation, 0.0f);
    }

    void renderer::draw_filled_rect(float x, float y, float w, float h, float radius, glm::vec4 color, float rotation) {
        draw_filled_rect(x, y, w, h, glm::vec4(radius), color, rotation);
    }

    void renderer::draw_rect_outline(float x, float y, float w, float h, glm::vec4 radii, glm::vec4 color, float stroke_width, float rotation) {
        tess.current_texture = nullptr;
        glm::vec4 colors[4] = { color, color, color, color };
        push_rect_ex(x, y, w, h, radii, colors, 0.0f, rotation, stroke_width);
    }

    void renderer::draw_rect_outline(float x, float y, float w, float h, float radius, glm::vec4 color, float stroke_width, float rotation) {
        tess.current_texture = nullptr;
        glm::vec4 colors[4] = { color, color, color, color };
        push_rect_ex(x, y, w, h, glm::vec4(radius), colors, 0.0f, rotation, stroke_width);
    }

    void renderer::draw_gradient_rect(float x, float y, float w, float h, float radius, glm::vec4 col_tl, glm::vec4 col_tr, glm::vec4 col_bl, glm::vec4 col_br, float rotation) {
        tess.current_texture = nullptr;
        glm::vec4 colors[4] = { col_tl, col_tr, col_br, col_bl };
        push_rect_gradient(x, y, w, h, radius, colors, 0.0f, rotation, -1.0f);
    }

    void renderer::draw_image(Resource res, float x, float y, float w, float h, float radius, glm::vec4 tint, float rotation) {
        tess.current_texture = get_or_create_texture(res);
        glm::vec4 colors[4] = { tint, tint, tint, tint };
        push_rect_gradient(x, y, w, h, radius, colors, 2.0f, rotation, 0.0f);
    }

    void renderer::draw_gif(Resource res, float x, float y, float w, float h, float radius) {
        auto gif = get_or_create_gif(res);
        if (!gif || gif->total_duration <= 0) return;

        static auto start_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time
        ).count();

        int loop_time = static_cast<int>(elapsed % gif->total_duration);

        int frame = 0;
        int current_accum = 0;

        for (int i = 0; i < gif->frame_count; i++) {
            current_accum += gif->delays[i];
            if (loop_time < current_accum) {
                frame = i;
                break;
            }
        }

        tess.current_texture = gif->srv.get();
        glm::vec4 white[4] = { {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1} };

        push_rect_gradient(x, y, w, h, radius, white, 2.0f, 0.0f, (float)frame);
    }

    void renderer::set_font(Resource tex_res, Resource json_res, bool aliased) {
        if (font_cache.count((const char*)tex_res.begin())) {
            current_font = &font_cache[(const char*)tex_res.begin()];
            return;
        }

        spdlog::debug("Initializing new font...");

        msdf_json data_json{};
        auto error = glz::read<glz::opts{.error_on_unknown_keys = false}>(
            data_json,
            std::string_view((char*)json_res.data(), json_res.size())
        );

        if (error) {
            std::string err_pretty = glz::format_error(error, std::string_view((char*)json_res.data(), json_res.size()));
            spdlog::error("Glaze JSON parsing failed: {}", err_pretty);
            return;
        }

        ID3D11ShaderResourceView* font_srv = get_or_create_texture(tex_res);
        if (!font_srv) {
            spdlog::error("Failed to create/retrieve font texture.");
            return;
        }

        font_data font;

        font.texture.copy_from(font_srv);

        font.px_range = data_json.atlas.distanceRange;
        font.prefers_aliased = aliased;

        float tw = (float)data_json.atlas.width;
        float th = (float)data_json.atlas.height;

        for (const auto& g : data_json.glyphs) {
            glyph& gl = font.glyphs[g.unicode];
            gl.advance = g.advance;

            if (g.planeBounds) {
                const auto& b = *g.planeBounds;
                gl.x0 = b.left;
                gl.y0 = b.bottom;
                gl.x1 = b.right;
                gl.y1 = b.top;
            } else {
                gl.x0 = gl.y0 = gl.x1 = gl.y1 = 0.0f;
            }

            if (g.atlasBounds) {
                const auto& b = *g.atlasBounds;
                gl.u0 = b.left / tw;
                gl.u1 = b.right / tw;

                gl.v0 = 1.0f - (b.top / th);
                gl.v1 = 1.0f - (b.bottom / th);
            }
        }

        const char* key = (const char*)tex_res.begin();
        font_cache[key] = std::move(font);
        current_font = &font_cache[key];

        spdlog::info("Font '{}' successfully loaded. Atlas: {}x{}, Glyphs: {}",
            key, data_json.atlas.width, data_json.atlas.height, data_json.glyphs.size());
    }

    void renderer::draw_text(std::string_view text, float x, float y, float size, glm::vec4 color, int aliased) {
        if (!current_font) return;

        tess.current_texture = current_font->texture.get();

        bool use_aliased = (aliased == -1) ? current_font->prefers_aliased : (bool)aliased;
        float cur_x = use_aliased ? std::floor(x + 0.5f) : x;
        float cur_y = use_aliased ? std::floor(y + 0.5f) : y;

        glm::vec4 cols[4] = { color, color, color, color };
        float aa_flag = use_aliased ? 1.0f : 0.0f;
        float effective_size = use_aliased ? std::round(size) : size;

        float first_glyph_left = 0.0f;
        bool first_glyph = true;

        for (size_t i = 0; i < text.length();) {
            uint32_t c = decode_utf8(text, i);

            if (!current_font->glyphs.count(c)) continue;
            const auto& g = current_font->glyphs.at(c);

            if (first_glyph) {
                first_glyph_left = g.x0 * effective_size;
                cur_x -= first_glyph_left;
                first_glyph = false;
            }

            float gx = cur_x + g.x0 * effective_size;
            float gy = cur_y - g.y1 * effective_size;
            float gw = (g.x1 - g.x0) * effective_size;
            float gh = (g.y1 - g.y0) * effective_size;

            if (use_aliased) {
                float snapped_gx = std::floor(gx + 0.5f);
                float snapped_gy = std::floor(gy + 0.5f);
                float snapped_gx2 = std::floor(gx + gw + 0.5f);
                float snapped_gy2 = std::floor(gy + gh + 0.5f);

                gx = snapped_gx;
                gy = snapped_gy;
                gw = snapped_gx2 - snapped_gx;
                gh = snapped_gy2 - snapped_gy;
            }

            push_rect_gradient(gx, gy, gw, gh, 0.0f, cols, 3.0f, 0.0f, aa_flag);

            auto& v = tess.vertices;
            size_t v_idx = v.size() - 6;

            v[v_idx+0].uv = {g.u0, g.v0};
            v[v_idx+1].uv = {g.u1, g.v0};
            v[v_idx+2].uv = {g.u0, g.v1};
            v[v_idx+3].uv = {g.u1, g.v0};
            v[v_idx+4].uv = {g.u1, g.v1};
            v[v_idx+5].uv = {g.u0, g.v1};

            float advance = g.advance * size;
            cur_x += use_aliased ? std::floor(advance + 0.5f) : advance;
        }
    }

    glm::vec2 renderer::get_text_size(std::string_view text, float size, int aliased) {
        if (!current_font || text.empty()) return {0.0f, 0.0f};

        bool use_aliased = (aliased == -1) ? current_font->prefers_aliased : (bool)aliased;
        float cur_x = use_aliased ? std::floor(0.5f) : 0.0f;
        float cur_y = 0.0f;

        float min_x = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::lowest();
        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::lowest();

        float first_glyph_left = 0.0f;
        bool first_glyph = true;

        for (size_t i = 0; i < text.length();) {
            uint32_t c = decode_utf8(text, i);
            auto it = current_font->glyphs.find(c);
            if (it == current_font->glyphs.end()) continue;

            const auto& g = it->second;

            if (first_glyph) {
                first_glyph_left = g.x0 * size;
                cur_x -= first_glyph_left;
                first_glyph = false;
            }

            float gx = cur_x + g.x0 * size;
            float gy = cur_y - g.y1 * size;
            float gw = (g.x1 - g.x0) * size;
            float gh = (g.y1 - g.y0) * size;

            if (use_aliased) {
                float snapped_gx = std::floor(gx + 0.5f);
                float snapped_gy = std::floor(gy + 0.5f);
                float snapped_gx2 = std::floor(gx + gw + 0.5f);
                float snapped_gy2 = std::floor(gy + gh + 0.5f);

                gx = snapped_gx;
                gy = snapped_gy;
                gw = snapped_gx2 - snapped_gx;
                gh = snapped_gy2 - snapped_gy;
            }

            min_x = std::min(min_x, gx);
            max_x = std::max(max_x, gx + gw);
            min_y = std::min(min_y, gy);
            max_y = std::max(max_y, gy + gh);

            float advance = g.advance * size;
            cur_x += use_aliased ? std::floor(advance + 0.5f) : advance;
        }

        if (min_x > max_x || min_y > max_y) return {0.0f, 0.0f};
        return { max_x - min_x, max_y - min_y };
    }

    void renderer::init(ID3D11Device* p_device) {
        device.copy_from(p_device);
        device->GetImmediateContext(ctx.put());

        winrt::com_ptr<ID3DBlob> vb_blob, pb_blob;
        D3DCompile(shaders::geometry::vertex, strlen(shaders::geometry::vertex), 0, 0, 0, "main", "vs_5_0", 0, 0, vb_blob.put(), 0);
        D3DCompile(shaders::geometry::pixel, strlen(shaders::geometry::pixel), 0, 0, 0, "main", "ps_5_0", 0, 0, pb_blob.put(), 0);

        device->CreateVertexShader(vb_blob->GetBufferPointer(), vb_blob->GetBufferSize(), 0, vs.put());
        device->CreatePixelShader(pb_blob->GetBufferPointer(), pb_blob->GetBufferSize(), 0, ps.put());

        D3D11_INPUT_ELEMENT_DESC input_layout[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"DATA",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"RADII",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        device->CreateInputLayout(input_layout, 5, vb_blob->GetBufferPointer(), vb_blob->GetBufferSize(), layout.put());

        D3D11_BUFFER_DESC cb_desc{};
        cb_desc.ByteWidth      = sizeof(DirectX::XMMATRIX);
        cb_desc.Usage          = D3D11_USAGE_DYNAMIC;
        cb_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device->CreateBuffer(&cb_desc, 0, constant_buffer.put());

        D3D11_BLEND_DESC b_desc{};
        b_desc.RenderTarget[0].BlendEnable           = TRUE;
        b_desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        b_desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        b_desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        b_desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        b_desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
        b_desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        b_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&b_desc, blend_enabled.put());

        D3D11_SAMPLER_DESC s_desc{};
        s_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        s_desc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
        s_desc.MipLODBias     = -0.75f;
        s_desc.MaxLOD         = D3D11_FLOAT32_MAX;
        device->CreateSamplerState(&s_desc, sampler.put());

        D3D11_RASTERIZER_DESC r_desc{};
        r_desc.FillMode = D3D11_FILL_SOLID;
        r_desc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState(&r_desc, raster_state.put());
    }

    void renderer::render_batch(float screen_w, float screen_h) {
        if (tess.vertices.empty()) return;

        v_buffer.update(device.get(), ctx.get(), tess.vertices);

        D3D11_VIEWPORT vp{ 0.0f, 0.0f, screen_w, screen_h, 0.0f, 1.0f };
        ctx->RSSetViewports(1, &vp);
        ctx->RSSetState(raster_state.get());

        D3D11_MAPPED_SUBRESOURCE res;
        if (SUCCEEDED(ctx->Map(constant_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res))) {
            *static_cast<DirectX::XMMATRIX*>(res.pData) = DirectX::XMMatrixTranspose(
                DirectX::XMMatrixOrthographicOffCenterLH(0, screen_w, screen_h, 0, 0, 1)
            );
            ctx->Unmap(constant_buffer.get(), 0);
        }

        UINT stride = sizeof(vertex), offset = 0;
        ID3D11Buffer* vbs[] = { v_buffer.get() };
        ctx->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
        ctx->IASetInputLayout(layout.get());
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ctx->VSSetShader(vs.get(), 0, 0);
        ID3D11Buffer* cb = constant_buffer.get();
        ctx->VSSetConstantBuffers(0, 1, &cb);

        ctx->PSSetSamplers(0, 1, sampler.put());
        ctx->PSSetShader(ps.get(), 0, 0);

        uint32_t current_v = 0;
        ctx->OMSetBlendState(blend_enabled.get(), nullptr, 0xFFFFFFFF);

        for (const auto& cmd : tess.commands) {
            ctx->PSSetShaderResources(0, 1, &cmd.texture);
            ctx->Draw(cmd.count, current_v);
            current_v += cmd.count;
        }

        tess.clear();
    }
}