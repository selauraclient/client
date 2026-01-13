#pragma once
#include "sgfx.hpp"
#include <d3d11.h>
#include <dxgi1_6.h>
#include <winrt/base.h>
#include <DirectXMath.h>

namespace sgfx {
    class renderer_d3d11 : public backend_interface {
    public:
        bool init(void* native_swapchain) override;
        void shutdown() override;
        void render(const draw_data& data) override;
        void create_texture(void* data, int width, int height, void** out_srv) override;
        void destroy_texture(texture_id handle) override;
        void create_shader() override;
        glm::vec2 get_screen_size() const override { return screen_size; }

    private:
        void create_blur_resources(const D3D11_TEXTURE2D_DESC& bb_desc);

        winrt::com_ptr<IDXGISwapChain3> swapchain;
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> ctx;
        winrt::com_ptr<ID3D11RenderTargetView> rtv;

        winrt::com_ptr<ID3D11Buffer> v_buffer;
        winrt::com_ptr<ID3D11Buffer> i_buffer;
        winrt::com_ptr<ID3D11Buffer> constant_buffer;

        winrt::com_ptr<ID3D11VertexShader> vs;
        winrt::com_ptr<ID3D11PixelShader> ps;
        winrt::com_ptr<ID3D11InputLayout> layout;

        winrt::com_ptr<ID3D11SamplerState> sampler;
        winrt::com_ptr<ID3D11BlendState> blend_state;
        winrt::com_ptr<ID3D11RasterizerState> raster_scissor_off;
        winrt::com_ptr<ID3D11RasterizerState> raster_scissor_on;
        winrt::com_ptr<ID3D11DepthStencilState> depth_disabled_state;

        winrt::com_ptr<ID3D11Texture2D> blur_textures[2];
        winrt::com_ptr<ID3D11ShaderResourceView> blur_srvs[2];
        winrt::com_ptr<ID3D11RenderTargetView> blur_rtvs[2];

        size_t v_capacity = 0;
        size_t i_capacity = 0;
        glm::vec2 screen_size = { 0, 0 };
    };
}