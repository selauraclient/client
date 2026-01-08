#include <pch.hpp>
#include <backends/imgui_impl_win32.h>

#include <core/service_manager.hpp>
#include <memory/detour.hpp>
#include <memory/hook.hpp>
#include <sdk/core/GameInput_GDK.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace selaura {
    static WNDPROC oWndProc;

    inline LRESULT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto& input_manager = selaura::get<selaura::input_manager>();
        if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) {
            input_manager.update(uMsg, wParam, lParam);
        }

        if (input_manager.is_input_cancelled()) {
            switch (uMsg) {
            case WM_LBUTTONDOWN: case WM_LBUTTONUP:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP:
            case WM_MOUSEMOVE:
            case WM_MOUSEWHEEL:
            case WM_KEYDOWN: case WM_SYSKEYDOWN:
                return TRUE;
            }
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
struct selaura::detour<&IGameInputReading::GetMouseState> {
    static bool WINAPI hk(IGameInputReading* thisptr, GameInputMouseState* state) {
        bool result = selaura::hook<&IGameInputReading::GetMouseState>::call(thisptr, state);

        auto& input_manager = selaura::get<selaura::input_manager>();
        input_manager.update(*state);

        if (input_manager.is_input_cancelled()) {
            state->buttons = GameInputMouseNone;

            state->absolutePositionX = input_manager.get_last_mouse_pos().x;
            state->absolutePositionY = input_manager.get_last_mouse_pos().y;
            state->positionX = 0;
            state->positionY = 0;
            state->wheelX = 0;
            state->wheelY = 0;
        }

        return result;
    }
};

template<>
struct selaura::detour<&IGameInput::GetCurrentReading> {
    static HRESULT WINAPI hk(IGameInput* thisptr, GameInputKind inputKind, IGameInputDevice* device, IGameInputReading** reading) {
        HRESULT hr = selaura::hook<&IGameInput::GetCurrentReading>::call(thisptr, inputKind, device, reading);

        static bool once = false;
        if (!once) {
            selaura::hook<&IGameInputReading::GetMouseState>::enable(*reading, 14);
            once = true;
        }

        return hr;
    }
};