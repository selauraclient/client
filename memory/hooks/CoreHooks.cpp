#include <pch.hpp>

template <>
struct selaura::detour<&MinecraftGame::_update> {
    static void hk(MinecraftGame* thisptr) {
        selaura::mcgame_update ev{};
        selaura::event_manager->dispatch(ev);

        return selaura::hook<&MinecraftGame::_update>::call(thisptr);
    }
};