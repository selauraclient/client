#include "main.hpp"

using namespace logger;

void init(std::span<std::byte> bytes) {
	logger::init();
	info("loaded selaura");
}

#ifdef _WIN32
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
        HMODULE hMinecraft = GetModuleHandleA("Minecraft.Windows.exe");
        if (hMinecraft == nullptr) return FALSE;

		DisableThreadLibraryCalls(hModule);
		try {
            MODULEINFO modInfo;
            if (GetModuleInformation(GetCurrentProcess(), hMinecraft, &modInfo, sizeof(MODULEINFO))) {
                std::span<std::byte> game_bytes(
                    reinterpret_cast<std::byte*>(modInfo.lpBaseOfDll),
                    modInfo.SizeOfImage
                );

                std::thread([=]() {
                    init(game_bytes);
                }).detach();
            }
            else {
                return FALSE;
            }
        } catch (...) {
            return FALSE;
        }
	}

	return TRUE;
}
#endif

#ifdef PLATFORM_LINUX
extern "C" [[gnu::visibility("default")]] void mod_preinit() {
    // credits to: https://github.com/CrackedMatter/mcpelauncher-zoom/blob/main/src/main.cpp
    auto gameWindowLib = dlopen("libmcpelauncher_gamewindow.so", 0);

    getPrimaryWindow = reinterpret_cast<decltype(getPrimaryWindow)>(dlsym(gwLib, "game_window_get_primary_window"));
    isMouseLocked = reinterpret_cast<decltype(isMouseLocked)>(dlsym(gwLib, "game_window_is_mouse_locked"));
    addKeyboardCallback = reinterpret_cast<decltype(addKeyboardCallback)>(dlsym(gwLib, "game_window_add_keyboard_callback"));
    addMouseButtonCallback = reinterpret_cast<decltype(addMouseButtonCallback)>(dlsym(gwLib, "game_window_add_mouse_button_callback"));
    addMousePositionCallback = reinterpret_cast<decltype(addMouseButtonCallback)>(dlsym(gwLib, "game_window_add_mouse_position_callback"));
    addMouseScrollCallback = reinterpret_cast<decltype(addMouseScrollCallback)>(dlsym(gwLib, "game_window_add_mouse_scroll_callback"));
    addWindowCreationCallback = reinterpret_cast<decltype(addWindowCreationCallback)>(dlsym(gwLib, "game_window_add_window_creation_callback"));

    auto menuLib = dlopen("libmcpelauncher_menu.so", 0);

    addMenu = reinterpret_cast<decltype(addMenu)>(dlsym(menuLib, "mcpelauncher_addmenu"));
    showWindow = reinterpret_cast<decltype(showWindow)>(dlsym(menuLib, "mcpelauncher_show_window"));
    closeWindow = reinterpret_cast<decltype(closeWindow)>(dlsym(menuLib, "mcpelauncher_close_window"));

    addWindowCreationCallback(nullptr, [](void*) {
        g_window = getPrimaryWindow();
        //addKeyboardCallback(g_window, nullptr, onKeyboard);
        //addMouseScrollCallback(g_window, nullptr, onMouseScroll);
    });
}

extern "C" [[gnu::visibility("default")]] void mod_init() {
    static std::span<std::byte> game_bytes;

    dl_iterate_phdr([](dl_phdr_info* info, size_t, void* mc) {
        auto h = dlopen(info->dlpi_name, RTLD_NOLOAD);
        dlclose(h);
        if (h == mc) {
            for (auto& phdr : std::span{ info->dlpi_phdr, info->dlpi_phnum }) {
                if (phdr.p_type == PT_LOAD && phdr.p_flags & PF_X) {
                    game_bytes = { reinterpret_cast<std::byte*>(info->dlpi_addr + phdr.p_vaddr), phdr.p_memsz };
                    return 1;
                }
            }
        }
        return 0;
        }, dlopen("libminecraftpe.so", 0));

    std::thread([=]() {
        init(game_bytes);
    }).detach();
}
#endif