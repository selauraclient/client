#include "win_utils.hpp"
#include "win_utils.hpp"
#include <winrt/base.h>

void win_utils::set_window_icon(HWND hwnd, const Resource& res) {
    using namespace Gdiplus;

    if (!original_icon_big) {
        original_icon_big = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
        original_icon_small = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);

        if (!original_icon_big) {
            original_icon_big = LoadIcon(GetModuleHandle(NULL), IDI_APPLICATION);
            original_icon_small = original_icon_big;
        }
    }

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    winrt::com_ptr<IStream> stream;
    stream.attach(SHCreateMemStream(reinterpret_cast<const BYTE*>(res.data()), static_cast<UINT>(res.size())));

    if (stream) {
        std::unique_ptr<Bitmap> bitmap(Bitmap::FromStream(stream.get()));

        if (bitmap && bitmap->GetLastStatus() == Ok) {
            HICON h_icon = nullptr;
            bitmap->GetHICON(&h_icon);

            if (h_icon) {
                SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(h_icon));
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(h_icon));

                SetClassLongPtr(hwnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(h_icon));
                SetClassLongPtr(hwnd, GCLP_HICONSM, reinterpret_cast<LONG_PTR>(h_icon));
            }
        }
    }

    GdiplusShutdown(gdiplusToken);
}

void win_utils::restore_original_state(HWND hwnd) {
    if (original_title[0] != '\0') {
        SetWindowTextA(hwnd, original_title);
    }

    if (original_icon_big) {
        SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(original_icon_big));
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(original_icon_small));

        SetClassLongPtr(hwnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(original_icon_big));
        SetClassLongPtr(hwnd, GCLP_HICONSM, reinterpret_cast<LONG_PTR>(original_icon_small));

        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

void win_utils::set_window_title(HWND hwnd, const std::string& title) {
    if (original_title[0] == '\0') {
        GetWindowTextA(hwnd, original_title, sizeof(original_title));
    }
    SetWindowTextA(hwnd, title.c_str());
}

std::string win_utils::get_formatted_version() {
    char file_path[MAX_PATH];
    GetModuleFileNameA(nullptr, file_path, MAX_PATH);

    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeA(file_path, &handle);
    if (size == 0) return "v0.0.0";

    std::vector<std::byte> buffer(size);
    if (!GetFileVersionInfoA(file_path, handle, size, buffer.data())) return "v0.0.0";

    VS_FIXEDFILEINFO* file_info = nullptr;
    UINT len = 0;
    if (!VerQueryValueA(buffer.data(), "\\", (LPVOID*)&file_info, &len)) return "v0.0.0";

    int major = HIWORD(file_info->dwFileVersionMS);
    int minor = LOWORD(file_info->dwFileVersionMS);
    int build = HIWORD(file_info->dwFileVersionLS);

    return fmt::format("v{}.{}.{}", major, minor, build);
}