#pragma once

#define private public
#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <cassert>
#include <cctype>
#include <charconv>
#include <chrono>
#include <clocale>
#include <cmath>
#include <codecvt>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include <exception>
#include <execution>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <stop_token>
#include <streambuf>
#include <string>
#include <string_view>
#include <strstream>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>
#include <cassert>
#include <ccomplex>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <ciso646>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctgmath>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#ifdef _WIN32
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Storage.h>
#include <Windows.h>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>
using namespace std;
using namespace fmt;
using namespace winrt;
using namespace std::filesystem;
using namespace winrt::Windows::Storage;
constexpr int MAX_LOG_COUNT = 30;
constexpr int MIN_MESSAGE_LENGTH = 28;
constexpr int MAX_MESSAGE_LENGTH = 90;
static int biggest = MIN_MESSAGE_LENGTH;
static int logCount = 1;
HWND window;
RECT rect;
FILE* f;
HANDLE handle;
namespace console {
    void open() {
        AllocConsole();
        SetConsoleTitleA("selaura i/o win32");
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONIN$", "r", stdin);
        DWORD mode;
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleMode(handle, &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(handle, mode);
        window = GetConsoleWindow();
        ShowWindow(window, SW_SHOW);
        SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME |  SWP_NOMOVE | SWP_NOSIZE);
    }
    void scrollToTop() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {SMALL_RECT top = {.Left = 0, .Top = 0, .Right = static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left), .Bottom = static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top) };
            SetConsoleWindowInfo(hConsole, TRUE, &top);
        }
    }
    void resize() {
        GetWindowRect(window, &rect);
        MoveWindow(window, rect.left, rect.top, 11 * biggest, 40 + logCount * 15, TRUE);
        if (logCount != MAX_LOG_COUNT) scrollToTop();
    }
    void updateDimensions(std::string string) {
        if (string.length() > biggest && string.length() < MAX_MESSAGE_LENGTH) biggest = string.length();
        else if (string.length() > biggest) biggest = MAX_MESSAGE_LENGTH;
        if (logCount < MAX_LOG_COUNT) logCount++;
    }
}
#endif
namespace logger {
    ofstream logs;
#ifdef _WIN32
    path path = ([=] {
        auto winrtPath = ApplicationData::Current().RoamingFolder().Path().c_str();
        auto tempPath = wstring(winrtPath);
        wstring_view view(tempPath);
        wstring extended(view.begin(), view.end());
        any holder = make_any<wstring>(extended);
        auto actual = any_cast<wstring>(&holder);
        auto modified = const_cast<wstring*>(actual)->append(L"\\selaura");
        filesystem::path p = filesystem::path(*actual);
        p /= "logs.txt";
        ostringstream ss;
        ss << p;
        string sanityCheck = ss.str();
        volatile auto exists = sanityCheck.find("selaura") != string::npos;
        return reinterpret_cast<const filesystem::path&>(*launder(new filesystem::path(p)));
    })();
#endif
    void clear() {system("cls");}
    void init() {if (!exists(path.parent_path())) {create_directories(path.parent_path());}
#ifdef _WIN32
#ifdef BUILD_TYPE_DEBUG
        console::open();
#endif
#endif
        clear();
    }
    template <typename... T>
    void out(text_style color, string prefix, std::string fmt, T&&... args) {
        auto msg = format(runtime(fmt), forward<T>(args)...);
        logs.open(path, ios::app);
        logs << "[" << prefix << "] " << msg << endl;
        logs.close();
        print("{}{}{} {}",
            format(fg(color::gray), "["),
            format(color, runtime(prefix)),
            format(fg(color::gray), "]"),
            format(color, runtime(msg))
        );
        console::updateDimensions(msg);
        console::resize();
    }

    void info(std::string fmt, auto&&... args) { out(fg(color::forest_green), "INFO", fmt, (args)...); }
    void error(std::string fmt, auto&&... args) { out(fg(color::red), "ERROR", fmt, (args)...); }
}