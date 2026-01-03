#pragma once
#include <pch.hpp>
#include <map>

namespace selaura {

    enum class blend_mode { normal, multiply };
    enum class draw_type : int { rect = 0, circle = 1, image = 2 };

    struct vertex {
        glm::vec3 pos;
        glm::vec4 color;
        glm::vec2 uv;
        glm::vec4 data;
        glm::vec4 radii;
    };

    struct draw_cmd {
        uint32_t count;
        blend_mode blend;
        ID3D11ShaderResourceView* texture;
    };

    struct tessellator {
        std::vector<vertex> vertices;
        std::vector<draw_cmd> commands;
        blend_mode current_blend = blend_mode::normal;
        ID3D11ShaderResourceView* current_texture = nullptr;

        void clear() {
            vertices.clear();
            commands.clear();
            current_blend = blend_mode::normal;
            current_texture = nullptr;
        }
    };

    class render_buffer {
        winrt::com_ptr<ID3D11Buffer> buffer;
        size_t capacity = 0;
    public:
        void update(ID3D11Device* device, ID3D11DeviceContext* ctx, const std::vector<vertex>& data);
        ID3D11Buffer* get() { return buffer.get(); }
    };

    struct gif_data {
        winrt::com_ptr<ID3D11ShaderResourceView> srv;
        int frame_count;
        std::vector<int> delays;
        int total_duration;
    };

    struct glyph {
        float x0, y0, x1, y1;
        float u0, v0, u1, v1;
        float advance;
    };

    struct font_data {
        winrt::com_ptr<ID3D11ShaderResourceView> texture;
        std::map<uint32_t, glyph> glyphs;
        float px_range;
    };

    class renderer {
        tessellator tess;
        render_buffer v_buffer;
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> ctx;
        winrt::com_ptr<ID3D11VertexShader> vs;
        winrt::com_ptr<ID3D11PixelShader> ps;
        winrt::com_ptr<ID3D11InputLayout> layout;
        winrt::com_ptr<ID3D11Buffer> constant_buffer;
        winrt::com_ptr<ID3D11DepthStencilState> depth_disabled;
        winrt::com_ptr<ID3D11BlendState> blend_enabled, blend_multiply;
        winrt::com_ptr<ID3D11RasterizerState> raster_state;
        winrt::com_ptr<ID3D11SamplerState> sampler;

        std::map<const char*, winrt::com_ptr<ID3D11ShaderResourceView>> texture_cache;
        std::map<const char*, gif_data> gif_cache;
        std::map<const char*, font_data> font_cache;
        font_data* current_font = nullptr;

        struct MatrixBuffer { DirectX::XMMATRIX projection; };

        glm::vec4 normalize_color(glm::vec4 c);
        void push_rect_gradient(float x, float y, float w, float h, float radius, glm::vec4 colors[4], float type, float rotation, float param);
        ID3D11ShaderResourceView* get_or_create_texture(Resource res);
        gif_data* get_or_create_gif(Resource res);

    public:
        void init(ID3D11Device* p_device);
        void set_blend_mode(blend_mode mode) { tess.current_blend = mode; }
        void push_rect_ex(float x, float y, float w, float h, glm::vec4 radii, glm::vec4 colors[4], float type, float rotation, float stroke_width);
        void draw_filled_rect(float x, float y, float w, float h, glm::vec4 radii, glm::vec4 color, float rotation = 0.0f);
        void draw_filled_rect(float x, float y, float w, float h, float radius, glm::vec4 color, float rotation = 0.0f);
        void draw_rect_outline(float x, float y, float w, float h, glm::vec4 radii, glm::vec4 color, float stroke_width, float rotation = 0.0f);
        void draw_rect_outline(float x, float y, float w, float h, float radius, glm::vec4 color, float stroke_width, float rotation = 0.0f);
        void draw_gradient_rect(float x, float y, float w, float h, float radius, glm::vec4 col_tl, glm::vec4 col_tr, glm::vec4 col_bl, glm::vec4 col_br, float rotation = 0.0f);
        void draw_image(Resource res, float x, float y, float w, float h, float radius = 0.0f, glm::vec4 tint = {255, 255, 255, 255}, float rotation = 0.0f);
        void draw_gif(Resource res, float x, float y, float w, float h, float radius = 0.0f);
        void set_font(Resource tex_res, Resource json_res);
        void draw_text(std::string_view text, float x, float y, float size, glm::vec4 color);
        glm::vec2 get_text_size(std::string_view text, float size);
        void render_batch(float screen_w, float screen_h);
    };
}