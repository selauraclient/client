#include "sgfx.hpp"
#include "renderer_d3d11.hpp"

#if defined(__clang__) && defined(_MSC_VER)
    #ifdef __cpuid
        #undef __cpuid
    #endif
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace sgfx {
    context& get_context() {
        static context ctx;
        return ctx;
    }

    bool init(void* native_swapchain) {
        auto& ctx = get_context();
        ctx.backend = std::make_unique<renderer_d3d11>();
        return ctx.backend->init(native_swapchain);
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
        if (ctx.backend) {
            ctx.backend->shutdown();
        }
    }

    void begin_frame(float width, float height) {
        auto& ctx = get_context();
        ctx.data.clear();
        ctx.data.display_size = { width, height };
        ctx.current_texture = nullptr;
        ctx.clip_enabled = false;
        ctx.current_clip = { 0, 0, 0, 0 };

        sgfx::draw_rect(0, 0, 0, 0, {0, 0, 0, 0});
    }

    void begin_frame(glm::vec2 size) {
        begin_frame(size.x, size.y);
    }

    void end_frame() {
        auto& ctx = get_context();
        if (ctx.backend && !ctx.data.vertices.empty()) {
            ctx.backend->render(ctx.data);
        }
    }

    void set_texture(texture_id tex) {
        get_context().current_texture = tex;
    }

    void set_clip(float x, float y, float w, float h) {
        auto& ctx = get_context();
        ctx.current_clip = { x, y, w, h };
        ctx.clip_enabled = true;
    }

    void reset_clip() {
        get_context().clip_enabled = false;
    }

    void context::add_command_if_needed() {
        bool needs_new = data.commands.empty() ||
                         data.commands.back().is_blur ||
                         data.commands.back().texture != current_texture ||
                         data.commands.back().clip_enabled != clip_enabled ||
                         (clip_enabled && data.commands.back().clip_rect != current_clip);
        if (needs_new) {
            data.commands.push_back({0, current_texture, current_clip, clip_enabled, false, 0.0f, 0});
        }
    }

    glm::vec4 normalize_col(glm::vec4 col) {
        if (col.r > 1.0f || col.g > 1.0f || col.b > 1.0f) {
            return { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a };
        }
        return col;
    }

    void draw_rect(float x, float y, float w, float h, glm::vec4 col, glm::vec4 radii) {
        auto& ctx = get_context();
        ctx.current_texture = nullptr;
        ctx.add_command_if_needed();

        glm::vec4 final_col = normalize_col(col);
        glm::vec4 p = { w, h, 0.0f, 0.0f };

        ctx.data.vertices.push_back({{x, y, 0}, final_col, {0, 0}, p, radii});
        ctx.data.vertices.push_back({{x+w, y, 0}, final_col, {1, 0}, p, radii});
        ctx.data.vertices.push_back({{x+w, y+h, 0}, final_col, {1, 1}, p, radii});
        ctx.data.vertices.push_back({{x, y+h, 0}, final_col, {0, 1}, p, radii});
        ctx.data.commands.back().count += 6;
    }

    void draw_rect_textured(float x, float y, float w, float h, texture_id tex, glm::vec4 col, glm::vec4 radii) {
        auto& ctx = get_context();
        ctx.current_texture = tex;
        ctx.add_command_if_needed();

        glm::vec4 final_col = normalize_col(col);
        glm::vec4 p = { w, h, 0.0f, 2.0f };

        ctx.data.vertices.push_back({{x, y, 0}, final_col, {0, 0}, p, radii});
        ctx.data.vertices.push_back({{x+w, y, 0}, final_col, {1, 0}, p, radii});
        ctx.data.vertices.push_back({{x+w, y+h, 0}, final_col, {1, 1}, p, radii});
        ctx.data.vertices.push_back({{x, y+h, 0}, final_col, {0, 1}, p, radii});
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

        float sw = ctx.data.display_size.x;
        float sh = ctx.data.display_size.y;
        float u0 = x / sw;
        float v0 = y / sh;
        float u1 = (x + w) / sw;
        float v1 = (y + h) / sh;

        glm::vec4 p = { w, h, 0.0f, 4.0f };

        ctx.data.vertices.push_back({{x, y, 0},     {1,1,1,1}, {u0, v0}, p, radii});
        ctx.data.vertices.push_back({{x+w, y, 0},   {1,1,1,1}, {u1, v0}, p, radii});
        ctx.data.vertices.push_back({{x+w, y+h, 0}, {1,1,1,1}, {u1, v1}, p, radii});
        ctx.data.vertices.push_back({{x, y+h, 0},   {1,1,1,1}, {u0, v1}, p, radii});

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
        if (font_cache.count(key)) {
            current_font = &font_cache[key];
            return;
        }
        msdf_json data_json{};
        auto error = glz::read<glz::opts{.error_on_unknown_keys = false}>(data_json, std::string_view((char*)json_res.data(), json_res.size()));
        if (error) return;
        texture_id font_tex = nullptr;
        int tw, th, ch;
        unsigned char* pixels = stbi_load_from_memory((const stbi_uc*)tex_res.data(), (int)tex_res.size(), &tw, &th, &ch, 4);
        if (pixels) {
            get_context().backend->create_texture(pixels, tw, th, &font_tex);
            stbi_image_free(pixels);
        }
        font_data font;
        font.tex_id = font_tex;
        font.px_range = data_json.atlas.distanceRange;
        font.prefers_aliased = aliased;
        float f_tw = (float)data_json.atlas.width;
        float f_th = (float)data_json.atlas.height;
        for (const auto& g : data_json.glyphs) {
            glyph& gl = font.glyphs[g.unicode];
            gl.advance = g.advance;
            if (g.planeBounds) {
                gl.x0 = g.planeBounds->left; gl.y0 = g.planeBounds->bottom;
                gl.x1 = g.planeBounds->right; gl.y1 = g.planeBounds->top;
            }
            if (g.atlasBounds) {
                gl.u0 = g.atlasBounds->left / f_tw; gl.u1 = g.atlasBounds->right / f_tw;
                gl.v0 = 1.0f - (g.atlasBounds->top / f_th); gl.v1 = 1.0f - (g.atlasBounds->bottom / f_th);
            }
        }
        font_cache[key] = std::move(font);
        current_font = &font_cache[key];
    }

    void draw_text(std::string_view text, float x, float y, float size, glm::vec4 color, int aliased) {
        if (!current_font) return;
        auto& ctx = get_context();
        ctx.current_texture = current_font->tex_id;
        ctx.add_command_if_needed();
        bool use_aliased = (aliased == -1) ? current_font->prefers_aliased : (bool)aliased;
        float cur_x = use_aliased ? std::floor(x + 0.5f) : x;
        float cur_y = use_aliased ? std::floor(y + 0.5f) : y;
        float aa_flag = use_aliased ? 1.0f : 0.0f;
        float effective_size = use_aliased ? std::round(size) : size;
        bool first_glyph = true;
        for (size_t i = 0; i < text.length(); ) {
            uint32_t c = decode_utf8(text, i);
            if (!current_font->glyphs.count(c)) continue;
            const auto& g = current_font->glyphs.at(c);
            if (first_glyph) {
                cur_x -= g.x0 * effective_size;
                first_glyph = false;
            }
            float gx = cur_x + g.x0 * effective_size;
            float gy = cur_y - g.y1 * effective_size;
            float gw = (g.x1 - g.x0) * effective_size;
            float gh = (g.y1 - g.y0) * effective_size;
            if (use_aliased) {
                float sx = std::floor(gx + 0.5f);
                float sy = std::floor(gy + 0.5f);
                gw = std::floor(gx + gw + 0.5f) - sx;
                gh = std::floor(gy + gh + 0.5f) - sy;
                gx = sx; gy = sy;
            }
            glm::vec4 shader_params = { gw, gh, (float)current_font->px_range, 3.0f };
            shader_params.y = aa_flag;
            ctx.data.vertices.push_back({ {gx, gy, 0}, color, {g.u0, g.v0}, shader_params, {0,0,0,0} });
            ctx.data.vertices.push_back({ {gx+gw, gy, 0}, color, {g.u1, g.v0}, shader_params, {0,0,0,0} });
            ctx.data.vertices.push_back({ {gx+gw, gy+gh, 0}, color, {g.u1, g.v1}, shader_params, {0,0,0,0} });
            ctx.data.vertices.push_back({ {gx, gy+gh, 0}, color, {g.u0, g.v1}, shader_params, {0,0,0,0} });
            ctx.data.commands.back().count += 6;
            float advance = g.advance * size;
            cur_x += use_aliased ? std::floor(advance + 0.5f) : advance;
        }
    }

    glm::vec2 get_text_size(std::string_view text, float size, int aliased) {
        if (!current_font || text.empty()) return {0.0f, 0.0f};
        bool use_aliased = (aliased == -1) ? current_font->prefers_aliased : (bool)aliased;
        float cur_x = 0.0f;
        float min_x = 1e10f, max_x = -1e10f, min_y = 1e10f, max_y = -1e10f;
        bool first_glyph = true;
        for (size_t i = 0; i < text.length(); ) {
            uint32_t c = decode_utf8(text, i);
            if (!current_font->glyphs.count(c)) continue;
            const auto& g = current_font->glyphs.at(c);
            if (first_glyph) {
                cur_x -= g.x0 * size;
                first_glyph = false;
            }
            float gx = cur_x + g.x0 * size;
            float gy = -g.y1 * size;
            float gw = (g.x1 - g.x0) * size;
            float gh = (g.y1 - g.y0) * size;
            min_x = std::min(min_x, gx);
            max_x = std::max(max_x, gx + gw);
            min_y = std::min(min_y, gy);
            max_y = std::max(max_y, gy + gh);
            float advance = g.advance * size;
            cur_x += use_aliased ? std::floor(advance + 0.5f) : advance;
        }
        return { max_x - min_x, max_y - min_y };
    }
}