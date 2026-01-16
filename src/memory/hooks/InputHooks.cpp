#include <pch.hpp>
#include <backends/imgui_impl_win32.h>

#include <core/service_manager.hpp>
#include <memory/detour.hpp>
#include <memory/hook.hpp>
#include <sdk/core/GameInput_GDK.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace selaura {
    static WNDPROC oWndProc;
    static std::bitset<256> global_keys_curr;
    static std::bitset<256> global_keys_prev;
    static bool was_cancelled_last_frame = false;

    inline LRESULT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        input_event ev{};
        ev.is_game_input = false;

        if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) {
            uint32_t vk = static_cast<uint32_t>(wParam);
            if (vk == VK_SHIFT) {
                uint32_t scancode = (lParam & 0x00ff0000) >> 16;
                vk = static_cast<uint32_t>(MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX));
            } else if (vk == VK_CONTROL) {
                vk = (lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
            } else if (vk == VK_MENU) {
                vk = (lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
            }

            if (vk < 256) {
                global_keys_prev = global_keys_curr;
                if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) global_keys_curr.set(vk, true);
                else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP) global_keys_curr.set(vk, false);

                ev.key = vk;
                ev.is_down = global_keys_curr.test(vk);
                ev.keys_curr = global_keys_curr;
                ev.keys_prev = global_keys_prev;
            }
        }

        selaura::get<selaura::event_manager>().dispatch(ev);

        if (ev.cancelled) {
            was_cancelled_last_frame = true;
            if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return TRUE;
            switch (uMsg) {
                case WM_INPUT:
                case WM_LBUTTONDOWN: case WM_LBUTTONUP:
                case WM_RBUTTONDOWN: case WM_RBUTTONUP:
                case WM_MBUTTONDOWN: case WM_MBUTTONUP:
                case WM_MOUSEMOVE:
                case WM_MOUSEWHEEL:
                case WM_KEYDOWN: case WM_SYSKEYDOWN:
                case WM_KEYUP: case WM_SYSKEYUP:
                    return TRUE;
            }
        } else {
            was_cancelled_last_frame = false;
        }

        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return TRUE;
        return CallWindowProc(selaura::oWndProc, hWnd, uMsg, wParam, lParam);
    }

    inline void init_wndproc(HWND mWindow) {
        oWndProc = (WNDPROC)SetWindowLongPtr(mWindow, GWLP_WNDPROC, (LONG_PTR)wnd_proc);
    }

    inline void remove_wndproc(HWND mWindow) {
        SetWindowLongPtr(mWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }
}

template<>
struct selaura::detour<&GameInput::v2::IGameInputReading::GetMouseState> {
    static bool WINAPI hk(GameInput::v2::IGameInputReading* thisptr, GameInput::v2::GameInputMouseState* state) {
        bool result = selaura::hook<&GameInput::v2::IGameInputReading::GetMouseState>::call(thisptr, state);

        static bool last_frame_was_cancelled = false;
        static uint32_t frozen_abs_x = 0;
        static uint32_t frozen_abs_y = 0;

        // Verify if this is an actual hardware reading
        bool has_movement = (state->positionX != 0 || state->positionY != 0);
        bool has_buttons  = (state->buttons != GameInput::v2::GameInputMouseNone);
        bool has_scroll   = (state->wheelX != 0 || state->wheelY != 0);
        bool has_abs_pos  = (state->absolutePositionX != 0 || state->absolutePositionY != 0);

        // If there is no data in this packet, ignore it entirely
        if (!has_movement && !has_buttons && !has_scroll && !has_abs_pos) {
            return result;
        }

        input_event ev{};
        ev.is_game_input = true;
        ev.mouse_pos = { (float)state->absolutePositionX, (float)state->absolutePositionY };
        ev.mouse_delta = { (float)state->positionX, (float)state->positionY };
        ev.scroll_wheel_delta = state->wheelY;
        ev.keys_curr = selaura::global_keys_curr;
        ev.keys_prev = selaura::global_keys_prev;

        selaura::get<selaura::event_manager>().dispatch(ev);

        if (ev.cancelled && !last_frame_was_cancelled) {
            frozen_abs_x = state->absolutePositionX;
            frozen_abs_y = state->absolutePositionY;
        }

        if (ev.cancelled || last_frame_was_cancelled) {
            state->buttons = GameInput::v2::GameInputMouseNone;
            state->positionX = 0;
            state->positionY = 0;
            state->wheelX = 0;
            state->wheelY = 0;

            if (frozen_abs_x != 0 || frozen_abs_y != 0) {
                state->absolutePositionX = frozen_abs_x;
                state->absolutePositionY = frozen_abs_y;
            }
        }

        last_frame_was_cancelled = ev.cancelled;
        return result;
    }
};

template<>
struct selaura::detour<&GameInput::v2::IGameInput::GetCurrentReading> {
    static HRESULT WINAPI hk(GameInput::v2::IGameInput* thisptr, GameInput::v2::GameInputKind inputKind, GameInput::v2::IGameInputDevice* device, GameInput::v2::IGameInputReading** reading) {
        HRESULT hr = selaura::hook<&GameInput::v2::IGameInput::GetCurrentReading>::call(thisptr, inputKind, device, reading);
        static bool once = false;
        if (!once && SUCCEEDED(hr) && reading && *reading) {
            selaura::hook<&GameInput::v2::IGameInputReading::GetMouseState>::enable(*reading, 14);
            once = true;
        }
        return hr;
    }
};