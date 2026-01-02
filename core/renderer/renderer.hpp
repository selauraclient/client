#pragma once
#include <pch.hpp>

namespace selaura {

    enum class blend_mode {
        normal,
        multiply
    };

    struct vertex {
        glm::vec3 pos;
        glm::vec4 color;
        glm::vec2 uv;
        glm::vec4 data;
    };

    struct draw_cmd {
        uint32_t count;
        bool blur;
        blend_mode blend;
    };

    struct tessellator {
        std::vector<vertex> vertices;
        std::vector<draw_cmd> commands;
        blend_mode current_blend = blend_mode::normal;
        void clear() { vertices.clear(); commands.clear(); current_blend = blend_mode::normal; }
    };

    class render_buffer {
        winrt::com_ptr<ID3D11Buffer> buffer;
        size_t capacity = 0;
    public:
        void update(ID3D11Device* device, ID3D11DeviceContext* ctx, const std::vector<vertex>& data);
        ID3D11Buffer* get() { return buffer.get(); }
    };

    class renderer_impl {
        tessellator tess;
        render_buffer v_buffer;
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> ctx;
        winrt::com_ptr<ID3D11VertexShader> vs;
        winrt::com_ptr<ID3D11PixelShader> ps, blur_ps;
        winrt::com_ptr<ID3D11InputLayout> layout;
        winrt::com_ptr<ID3D11Buffer> constant_buffer;
        winrt::com_ptr<ID3D11DepthStencilState> depth_disabled;
        winrt::com_ptr<ID3D11BlendState> blend_enabled;
        winrt::com_ptr<ID3D11BlendState> blend_multiply;
        winrt::com_ptr<ID3D11RasterizerState> raster_state;
        winrt::com_ptr<ID3D11SamplerState> sampler;

        struct MatrixBuffer { DirectX::XMMATRIX projection; };

        glm::vec4 normalize_color(glm::vec4 c);
        void push_rect_gradient(float x, float y, float w, float h, float radius, glm::vec4 colors[4], float type, float rotation);
    public:
        void init(ID3D11Device* p_device);
        void set_blend_mode(blend_mode mode) { tess.current_blend = mode; }
        void draw_filled_rect(float x, float y, float w, float h, float radius, glm::vec4 color, float rotation = 0.0f);
        void draw_gradient_rect(float x, float y, float w, float h, float radius, glm::vec4 col_tl, glm::vec4 col_tr, glm::vec4 col_bl, glm::vec4 col_br, float rotation = 0.0f);
        void draw_circle(float x, float y, float radius, glm::vec4 color, float rotation = 0.0f);
        void render_batch(float screen_w, float screen_h);
    };

    inline std::unique_ptr<renderer_impl> renderer;
}