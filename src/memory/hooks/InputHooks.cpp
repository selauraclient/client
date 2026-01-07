#include <pch.hpp>
#include <backends/imgui_impl_win32.h>

#include <core/service_manager.hpp>
#include <memory/detour.hpp>
#include <memory/hook.hpp>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace selaura {
    static WNDPROC oWndProc;

    inline LRESULT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) {
            selaura::get<selaura::input_manager>().update(uMsg, wParam, lParam);
        }

        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return TRUE;
        return CallWindowProc(selaura::oWndProc, hWnd, uMsg, wParam, lParam);
    };

    inline void init_wndproc(HWND mWindow) {
        oWndProc = (WNDPROC)SetWindowLongPtr(mWindow, GWLP_WNDPROC, (__int3264)(LONG_PTR)wnd_proc);
    }

    inline void remove_wndproc(HWND mWindow) {
        SetWindowLongPtr(mWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }
};

using namespace GameInput::v2;

template<>
struct selaura::detour<&IGameInput::GetCurrentReading> {
    static HRESULT WINAPI hk(IGameInput* thisptr, GameInputKind inputKind, IGameInputDevice* device, IGameInputReading** reading) {
        HRESULT hr = selaura::hook<&IGameInput::GetCurrentReading>::call(thisptr, inputKind, device, reading);
        if (SUCCEEDED(hr) && reading) {
            if (inputKind == GameInputKind::GameInputKindMouse) {
                auto& input_manager = selaura::get<selaura::input_manager>();

                GameInputMouseState state{};
                (*reading)->GetMouseState(&state);

                input_manager.update(state);
            }
        }
        return hr;
    }
};