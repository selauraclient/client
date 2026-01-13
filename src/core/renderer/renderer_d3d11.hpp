#pragma once
#include <pch.hpp>
#include "renderer_d3d.hpp"

namespace sgfx {
    struct renderer_d3d11 : public renderer_d3d {
        virtual bool init(void* sc) override;
        virtual void shutdown() override;
        virtual void render(const draw_data& data) override;
        virtual void create_texture(void* data, int width, int height, void** out_srv) override;
        virtual void destroy_texture(texture_id handle) override;
        virtual void create_shader() override;
    private:
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> ctx;
        winrt::com_ptr<ID3D11Texture2D> back_buffer;
        winrt::com_ptr<ID3D11RenderTargetView> rtv;

        winrt::com_ptr<ID3D11SamplerState> sampler;
        winrt::com_ptr<ID3D11BlendState> blend_state;
        winrt::com_ptr<ID3D11RasterizerState> raster_scissor_off;
        winrt::com_ptr<ID3D11RasterizerState> raster_scissor_on;
        winrt::com_ptr<ID3D11DepthStencilState> depth_disabled_state;

        winrt::com_ptr<ID3D11VertexShader> vs;
        winrt::com_ptr<ID3D11PixelShader> ps;
        winrt::com_ptr<ID3D11InputLayout> layout;
        winrt::com_ptr<ID3D11Buffer> v_buffer;
        winrt::com_ptr<ID3D11Buffer> i_buffer;
        winrt::com_ptr<ID3D11Buffer> constant_buffer;

        size_t v_capacity = 0;
        size_t i_capacity = 0;
    };
}