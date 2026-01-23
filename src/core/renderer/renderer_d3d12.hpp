#pragma once
#include "renderer_d3d11.hpp"
#include <d3d11on12.h>
#include <d3d12.h>

namespace sgfx {
    class renderer_d3d12 : public renderer_d3d11 {
    public:
        bool init(void* native_swapchain) override;
        void render(const draw_data& data) override;
        void shutdown() override;

        winrt::com_ptr<ID3D12CommandQueue> d3d12_queue;
    private:
        winrt::com_ptr<ID3D12Device> d12_device;
        winrt::com_ptr<ID3D11On12Device> d11on12_device;

        std::vector<winrt::com_ptr<ID3D11Resource>> wrapped_back_buffers;
        std::vector<winrt::com_ptr<ID3D11RenderTargetView>> wrapped_rtvs;
        uint32_t buffer_count = 0;
    };
}