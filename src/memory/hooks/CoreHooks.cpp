#include <pch.hpp>

#include <core/service_manager.hpp>
#include <memory/detour.hpp>
#include <memory/hook.hpp>
#include <sdk/core/MinecraftGame.hpp>

template <>
struct selaura::detour<&MinecraftGame::_update> {
    static void hk(MinecraftGame* thisptr) {
        //selaura::mc = thisptr;

        selaura::mcgame_update ev{};
        ev.mc = thisptr;
        selaura::get<selaura::event_manager>().dispatch(ev);

        return selaura::hook<&MinecraftGame::_update>::call(thisptr);
    }
};


template <>
struct selaura::detour<&MinecraftGame::onDeviceLost> {
    static void hk(MinecraftGame* thisptr) {
        auto& scrn = selaura::get<selaura::screen_manager>();
        if (scrn.any_screens_enabled()) return;
        return selaura::hook<&MinecraftGame::onDeviceLost>::call(thisptr);
    }
};