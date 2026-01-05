#include <pch.hpp>

#include <backends/imgui_impl_win32.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace selaura {
    static WNDPROC oWndProc;

    inline LRESULT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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