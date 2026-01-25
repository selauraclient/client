#include "sgfx.hpp"
#include "renderer_d3d11.hpp"
#include "renderer_d3d12.hpp"

#if defined(__clang__) && defined(_MSC_VER)
    #ifdef __cpuid
        #undef __cpuid
    #endif
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace sgfx {
    static ID3D12CommandQueue* global_d3d12_queue = nullptr;

    inline DirectX::XMFLOAT4 to_xm4(const glm::vec4& v) { return { v.x, v.y, v.z, v.w }; }
    inline DirectX::XMFLOAT3 to_xm3(const glm::vec3& v) { return { v.x, v.y, v.z }; }
    inline DirectX::XMFLOAT2 to_xm2(float x, float y) { return { x, y }; }

    void set_d3d12_queue(void* queue) {
        global_d3d12_queue = static_cast<ID3D12CommandQueue*>(queue);
    }

    context& get_context() {
        static context ctx;
        return ctx;
    }

    bool init(void* native_swapchain) {
        auto& ctx = get_context();
        auto* sc3 = static_cast<IDXGISwapChain3*>(native_swapchain);
        winrt::com_ptr<ID3D12Device> test_device;
        if (SUCCEEDED(sc3->GetDevice(IID_PPV_ARGS(test_device.put())))) {
            if (!global_d3d12_queue) return false;
            auto d12_backend = std::make_unique<renderer_d3d12>();
            d12_backend->d3d12_queue.copy_from(global_d3d12_queue);
            if (d12_backend->init(native_swapchain)) {
                ctx.backend = std::move(d12_backend);
                return true;
            }
            return false;
        } else {
            ctx.backend = std::make_unique<renderer_d3d11>();
            return ctx.backend->init(native_swapchain);
        }
    }

    void shutdown() {
        auto& ctx = get_context();
        if (ctx.backend) {
            ctx.backend->shutdown();
            ctx.backend.reset();
        }
    }

    void invalidate() {
        auto& ctx = get_context();
        if (ctx.backend) ctx.backend->shutdown();
    }

    void begin_frame(float width, float height) {
        auto& ctx = get_context();
        ctx.data.clear();
        ctx.data.display_size = { width, height };
        ctx.current_texture = nullptr;
        ctx.clip_enabled = false;
        ctx.current_clip = { 0, 0, 0, 0 };
    }

    void begin_frame(glm::vec2 size) { begin_frame(size.x, size.y); }

    void end_frame() {
        auto& ctx = get_context();
        if (ctx.backend && !ctx.data.vertices.empty()) ctx.backend->render(ctx.data);
    }

    void set_texture(texture_id tex) { get_context().current_texture = tex; }

    void set_clip(float x, float y, float w, float h) {
        auto& ctx = get_context();
        ctx.current_clip = { x, y, w, h };
        ctx.clip_enabled = true;
    }

    void reset_clip() { get_context().clip_enabled = false; }

    void context::add_command_if_needed() {
        bool needs_new = data.commands.empty() ||
                         data.commands.back().is_blur ||
                         data.commands.back().texture != current_texture ||
                         data.commands.back().clip_enabled != clip_enabled ||
                         (clip_enabled && data.commands.back().clip_rect != current_clip);
        if (needs_new) data.commands.push_back({0, current_texture, current_clip, clip_enabled, false, 0.0f, 0});
    }

    glm::vec4 normalize_col(glm::vec4 col) {
        if (col.r > 1.0f || col.g > 1.0f || col.b > 1.0f) return { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a };
        return col;
    }

    void draw_rect(float x, float y, float w, float h, glm::vec4 col, glm::vec4 radii) {
        auto& ctx = get_context();
        ctx.current_texture = nullptr;
        ctx.add_command_if_needed();
        DirectX::XMFLOAT4 fc = to_xm4(normalize_col(col));
        DirectX::XMFLOAT4 p = { w, h, 0.0f, 0.0f };
        DirectX::XMFLOAT4 r = to_xm4(radii);
        ctx.data.vertices.push_back({{x, y, 0}, fc, {0, 0}, p, r});
        ctx.data.vertices.push_back({{x+w, y, 0}, fc, {1, 0}, p, r});
        ctx.data.vertices.push_back({{x+w, y+h, 0}, fc, {1, 1}, p, r});
        ctx.data.vertices.push_back({{x, y+h, 0}, fc, {0, 1}, p, r});
        ctx.data.commands.back().count += 6;
    }

    void draw_rect_stroke(float x, float y, float w, float h, float thickness, glm::vec4 col, glm::vec4 radii) {
        auto& ctx = get_context();
        ctx.current_texture = nullptr;
        ctx.add_command_if_needed();

        DirectX::XMFLOAT4 fc = to_xm4(normalize_col(col));
        DirectX::XMFLOAT4 p = { w, h, thickness, 5.0f };
        DirectX::XMFLOAT4 r = to_xm4(radii);

        ctx.data.vertices.push_back({{x, y, 0}, fc, {0, 0}, p, r});
        ctx.data.vertices.push_back({{x+w, y, 0}, fc, {1, 0}, p, r});
        ctx.data.vertices.push_back({{x+w, y+h, 0}, fc, {1, 1}, p, r});
        ctx.data.vertices.push_back({{x, y+h, 0}, fc, {0, 1}, p, r});

        ctx.data.commands.back().count += 6;
    }

    void draw_rect_textured(float x, float y, float w, float h, texture_id tex, glm::vec4 col, glm::vec4 radii) {
        auto& ctx = get_context();
        ctx.current_texture = tex;
        ctx.add_command_if_needed();
        DirectX::XMFLOAT4 fc = to_xm4(normalize_col(col));
        DirectX::XMFLOAT4 p = { w, h, 0.0f, 2.0f };
        DirectX::XMFLOAT4 r = to_xm4(radii);
        ctx.data.vertices.push_back({{x, y, 0}, fc, {0, 0}, p, r});
        ctx.data.vertices.push_back({{x+w, y, 0}, fc, {1, 0}, p, r});
        ctx.data.vertices.push_back({{x+w, y+h, 0}, fc, {1, 1}, p, r});
        ctx.data.vertices.push_back({{x, y+h, 0}, fc, {0, 1}, p, r});
        ctx.data.commands.back().count += 6;
    }

    void draw_blur(float x, float y, float w, float h, float intensity, int iterations, glm::vec4 radii) {
        auto& ctx = get_context();
        draw_cmd cmd{};
        cmd.is_blur = true;
        cmd.blur_intensity = intensity;
        cmd.blur_iterations = iterations;
        cmd.count = 6;
        ctx.data.commands.push_back(cmd);
        float sw = ctx.data.display_size.x, sh = ctx.data.display_size.y;
        DirectX::XMFLOAT4 p = { w, h, 0.0f, 4.0f }, r = to_xm4(radii), c = {1,1,1,1};
        ctx.data.vertices.push_back({{x, y, 0}, c, {x/sw, y/sh}, p, r});
        ctx.data.vertices.push_back({{x+w, y, 0}, c, {(x+w)/sw, y/sh}, p, r});
        ctx.data.vertices.push_back({{x+w, y+h, 0}, c, {(x+w)/sw, (y+h)/sh}, p, r});
        ctx.data.vertices.push_back({{x, y+h, 0}, c, {x/sw, (y+h)/sh}, p, r});
        ctx.current_texture = reinterpret_cast<texture_id>(-1);
    }

    void draw_image(Resource res, float x, float y, float w, float h, glm::vec4 col, glm::vec4 radii) {
        auto& cache = image_cache[res.begin()];
        if (!cache.id) {
            int c;
            unsigned char* p = stbi_load_from_memory((const stbi_uc*)res.data(), (int)res.size(), &cache.width, &cache.height, &c, 4);
            if (p) { get_context().backend->create_texture(p, cache.width, cache.height, &cache.id); stbi_image_free(p); }
        }
        draw_rect_textured(x, y, w, h, cache.id, col, radii);
    }

    uint32_t decode_utf8(std::string_view text, size_t& i) {
        uint8_t c = (uint8_t)text[i++];
        if (c < 0x80) return c;
        if ((c & 0xE0) == 0xC0) return ((c & 0x1F) << 6) | (text[i++] & 0x3F);
        if ((c & 0xF0) == 0xE0) return ((c & 0x0F) << 12) | ((text[i++] & 0x3F) << 6) | (text[i++] & 0x3F);
        if ((c & 0xF8) == 0xF0) return ((c & 0x07) << 18) | ((text[i++] & 0x3F) << 12) | ((text[i++] & 0x3F) << 6) | (text[i++] & 0x3F);
        return 0;
    }

    void set_font(Resource tex_res, Resource json_res, bool aliased) {
        const char* key = (const char*)tex_res.begin();
        if (font_cache.count(key)) { current_font = &font_cache[key]; return; }
        msdf_json data_json{};
        if (glz::read<glz::opts{.error_on_unknown_keys = false}>(data_json, std::string_view((char*)json_res.data(), json_res.size()))) return;
        texture_id font_tex = nullptr;
        int tw, th, ch;
        unsigned char* pixels = stbi_load_from_memory((const stbi_uc*)tex_res.data(), (int)tex_res.size(), &tw, &th, &ch, 4);
        if (pixels) { get_context().backend->create_texture(pixels, tw, th, &font_tex); stbi_image_free(pixels); }
        font_data font;
        font.tex_id = font_tex; font.px_range = data_json.atlas.distanceRange; font.prefers_aliased = aliased;
        float f_tw = (float)data_json.atlas.width, f_th = (float)data_json.atlas.height;
        for (const auto& g : data_json.glyphs) {
            glyph& gl = font.glyphs[g.unicode]; gl.advance = g.advance;
            if (g.planeBounds) { gl.x0 = g.planeBounds->left; gl.y0 = g.planeBounds->bottom; gl.x1 = g.planeBounds->right; gl.y1 = g.planeBounds->top; }
            if (g.atlasBounds) { gl.u0 = g.atlasBounds->left / f_tw; gl.u1 = g.atlasBounds->right / f_tw; gl.v0 = 1.0f - (g.atlasBounds->top / f_th); gl.v1 = 1.0f - (g.atlasBounds->bottom / f_th); }
        }
        font_cache[key] = std::move(font);
        current_font = &font_cache[key];
    }

    void draw_text(std::string_view text, float x, float y, float size, glm::vec4 color, int aliased) {
        if (!current_font || text.empty()) return;
        auto& ctx = get_context();
        ctx.current_texture = current_font->tex_id;
        ctx.add_command_if_needed();
        bool use_aliased = (aliased == -1) ? current_font->prefers_aliased : (bool)aliased;
        float effective_size = use_aliased ? std::round(size) : size;
        float max_ascent = 0.0f;
        for (const auto& [unicode, g] : current_font->glyphs) max_ascent = std::max(max_ascent, g.y1);
        float cur_x = x, cur_y = y + (max_ascent * effective_size);
        if (use_aliased) { cur_x = std::floor(cur_x + 0.5f); cur_y = std::floor(cur_y + 0.5f); }
        DirectX::XMFLOAT4 c = to_xm4(color), r_null = {0,0,0,0};
        bool first_glyph = true;
        for (size_t i = 0; i < text.length(); ) {
            uint32_t code = decode_utf8(text, i);
            if (!current_font->glyphs.count(code)) continue;
            const auto& g = current_font->glyphs.at(code);
            if (first_glyph) { cur_x -= g.x0 * effective_size; first_glyph = false; }
            float gx = cur_x + g.x0 * effective_size, gy = cur_y - g.y1 * effective_size;
            float gw = (g.x1 - g.x0) * effective_size, gh = (g.y1 - g.y0) * effective_size;
            if (use_aliased) { float sx = std::floor(gx + 0.5f), sy = std::floor(gy + 0.5f); gw = std::floor(gx + gw + 0.5f) - sx; gh = std::floor(gy + gh + 0.5f) - sy; gx = sx; gy = sy; }
            DirectX::XMFLOAT4 p = { gw, gh, (float)current_font->px_range, 3.0f };
            p.y = use_aliased ? 1.0f : 0.0f;
            ctx.data.vertices.push_back({{gx, gy, 0}, c, {g.u0, g.v0}, p, r_null});
            ctx.data.vertices.push_back({{gx+gw, gy, 0}, c, {g.u1, g.v0}, p, r_null});
            ctx.data.vertices.push_back({{gx+gw, gy+gh, 0}, c, {g.u1, g.v1}, p, r_null});
            ctx.data.vertices.push_back({{gx, gy+gh, 0}, c, {g.u0, g.v1}, p, r_null});
            ctx.data.commands.back().count += 6;
            float advance = g.advance * effective_size;
            cur_x += use_aliased ? std::floor(advance + 0.5f) : advance;
        }
    }

    glm::vec2 get_text_size(std::string_view text, float size, int aliased) {
        if (!current_font || text.empty()) return {0.0f, 0.0f};

        bool use_aliased = (aliased == -1) ? current_font->prefers_aliased : (bool)aliased;
        float effective_size = use_aliased ? std::round(size) : size;

        float total_advance = 0.0f;
        float max_ascent = 0.0f;
        float max_descent = 0.0f;

        for (const auto& [unicode, g] : current_font->glyphs) {
            max_ascent = std::max(max_ascent, g.y1);
            max_descent = std::min(max_descent, g.y0);
        }

        bool first_glyph = true;
        for (size_t i = 0; i < text.length(); ) {
            uint32_t c = decode_utf8(text, i);
            if (!current_font->glyphs.count(c)) continue;

            const auto& g = current_font->glyphs.at(c);

            if (first_glyph) {
                total_advance -= g.x0 * effective_size;
                first_glyph = false;
            }

            float advance = g.advance * effective_size;
            total_advance += use_aliased ? std::floor(advance + 0.5f) : advance;
        }

        float height = (max_ascent - max_descent) * effective_size;

        return { total_advance, height };
    }
}