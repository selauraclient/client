#pragma once
#include <pch.hpp>

template<>
struct selaura::detour<&IDXGISwapChain::Present> {
    static HRESULT hk(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags);
};

template<>
struct selaura::detour<&IDXGISwapChain::ResizeBuffers> {
    static HRESULT hk(IDXGISwapChain* thisptr, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
};

template<>
struct selaura::detour<&bgfx::d3d11::RenderContextD3D11::submit> {
    static void hk(bgfx::d3d11::RenderContextD3D11* thisptr, void* a1, void* a2, void* a3);
};