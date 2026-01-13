#pragma once
#include <pch.hpp>

namespace sgfx {
    struct vertex {
        glm::vec3 pos;
        glm::vec4 col;
        glm::vec2 uv;
        glm::vec4 data;
        glm::vec4 radii;
    };

    struct draw_cmd {
        uint32_t count;
        void* texture;
        glm::vec4 clip_rect;
        bool clip_enabled;
    };

    struct draw_data {
        std::vector<vertex> vertices;
        std::vector<draw_cmd> commands;
        glm::vec2 display_size;

        void clear() {
            vertices.clear();
            commands.clear();
        }
    };

    using texture_id = void*;

    struct backend_interface {
        virtual ~backend_interface() = default;
        virtual bool init(void* native) = 0;
        virtual void shutdown() = 0;
        virtual void render(const draw_data& data) = 0;
        virtual void create_texture(void* data, int width, int height, void** out_srv) = 0;
        virtual void destroy_texture(texture_id handle) = 0;
        virtual void create_shader(const char* vs_src, const char* ps_src) = 0;
        [[nodiscard]] virtual glm::vec2 get_screen_size() const = 0;
    };

    struct context {
        std::unique_ptr<backend_interface> backend;
        draw_data data;

        void* current_texture = nullptr;
        glm::vec4 current_clip = { 0, 0, 0, 0 };
        bool clip_enabled = false;

        void add_command_if_needed();
    };

    context& get_context();

    bool init(void* native_swapchain);
    void shutdown();
    void invalidate();

    void begin_frame(float width, float height);
    void begin_frame(glm::vec2 size);
    void end_frame();

    void set_texture(texture_id tex);
    void set_clip(float x, float y, float w, float h);
    void reset_clip();

    void draw_rect(float x, float y, float w, float h, glm::vec4 col, glm::vec4 radii = {0,0,0,0});
    void draw_rect_textured(float x, float y, float w, float h, texture_id tex, glm::vec4 col = {1,1,1,1}, glm::vec4 radii = {0,0,0,0});

    struct texture_resource {
        texture_id id = nullptr;
        int width = 0;
        int height = 0;
    };

    struct gif_resource {
        std::vector<texture_id> frames;
        std::vector<int> delays;
        int width, height;
        float total_time = 0.0f;

        texture_id get_frame(float time) const;
    };

    inline std::map<const char*, texture_resource> image_cache;
    inline std::map<const char*, gif_resource> gif_cache;

    struct atlas_info {
        float distanceRange;
        float width;
        float height;
    };

    struct plane_bounds {
        float left, bottom, right, top;
    };

    struct atlas_bounds {
        float left, bottom, right, top;
    };

    struct glyph_entry {
        uint32_t unicode;
        float advance = 0.0f;
        std::optional<plane_bounds> planeBounds;
        std::optional<atlas_bounds> atlasBounds;
    };

    struct msdf_json {
        atlas_info atlas;
        std::vector<glyph_entry> glyphs;
    };

    struct glyph {
        float x0, y0, x1, y1;
        float u0, v0, u1, v1;
        float advance;
    };

    struct font_data {
        texture_id tex_id;
        std::map<uint32_t, glyph> glyphs;
        float px_range;
        bool prefers_aliased = false;
    };

    inline std::map<const char*, font_data> font_cache;
    inline font_data* current_font = nullptr;

    void draw_image(Resource res, float x, float y, float w, float h, glm::vec4 col = {1,1,1,1}, glm::vec4 radii = {0,0,0,0});
    void draw_gif(Resource res, float x, float y, float w, float h, float time, glm::vec4 col = {1,1,1,1}, glm::vec4 radii = {0,0,0,0});

    void set_font(Resource tex_res, Resource json_res, bool aliased = -1);
    void draw_text(std::string_view text, float x, float y, float size, glm::vec4 color = {1, 1, 1, 1}, int aliased = -1);
    glm::vec2 get_text_size(std::string_view text, float size, int aliased = -1);
}