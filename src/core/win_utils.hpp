#pragma once
#include <pch.hpp>

namespace win_utils {
    void set_window_icon(HWND hwnd, const Resource& res);
    void set_window_title(HWND hwnd, const std::string& title);
    std::string get_formatted_version();
    void restore_original_state(HWND hwnd);

    inline HICON original_icon_big = nullptr;
    inline HICON original_icon_small = nullptr;
    inline char original_title[256] = { 0 };
};