#pragma once
#include <pch.hpp>

template <>
struct selaura::detour<&WndProc> {
    static LRESULT hk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};