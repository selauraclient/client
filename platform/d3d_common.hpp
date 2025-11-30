#pragma once
#include <pch.hpp>

namespace Selaura::D3D {
    struct D3DContext {
        winrt::com_ptr<IDXGISwapChain3> mSwapChain; // abi compat with minecraft
    };

    struct D3D11Context : public D3DContext {
        winrt::com_ptr<ID3D11Device> mDevice;
        winrt::com_ptr<ID3D11DeviceContext> mDeviceContext;
    };

    struct D3D12Context : public D3DContext {
        winrt::com_ptr<ID3D12Device> mDevice;
        winrt::com_ptr<ID3D12CommandQueue> mCommandQueue;
        winrt::com_ptr<ID3D12GraphicsCommandList> mCommandList;
    };

    struct TemporaryWindow {
        TemporaryWindow() {
            this->mWindowClass.cbSize = sizeof(WNDCLASSEX);
			this->mWindowClass.style = CS_HREDRAW | CS_VREDRAW;
			this->mWindowClass.lpfnWndProc = DefWindowProc;
			this->mWindowClass.cbClsExtra = 0;
			this->mWindowClass.cbWndExtra = 0;
			this->mWindowClass.hInstance = GetModuleHandle(nullptr);
			this->mWindowClass.hIcon = nullptr;
			this->mWindowClass.hCursor = nullptr;
			this->mWindowClass.hbrBackground = nullptr;
			this->mWindowClass.lpszMenuName = nullptr;
			this->mWindowClass.lpszClassName = L"Selaura";
			this->mWindowClass.hIconSm = nullptr;
			::RegisterClassEx(&this->mWindowClass);

			this->mHandle = ::CreateWindow(
				this->mWindowClass.lpszClassName,
				_T("Selaura DirectX Window"),
				WS_OVERLAPPEDWINDOW,
				0, 0, 100, 100,
				nullptr,
				nullptr,
				this->mWindowClass.hInstance,
				nullptr
			);
        }

        ~TemporaryWindow() {
			::DestroyWindow(this->mHandle);
            ::UnregisterClass(this->mWindowClass.lpszClassName, this->mWindowClass.hInstance);
		}

        explicit operator HWND() const {
			return this->mHandle;
		}
    private:
        WNDCLASSEX mWindowClass{};
        HWND mHandle{};
    };
};