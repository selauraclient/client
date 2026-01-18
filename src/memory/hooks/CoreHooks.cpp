#include <pch.hpp>

#include <core/service_manager.hpp>
#include <memory/detour.hpp>
#include <memory/hook.hpp>
#include <sdk/core/MinecraftGame.hpp>

#include "sdk/renderer/ScreenView.hpp"

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

template <>
struct selaura::detour<&ScreenView::render> {
    static void hk(ScreenView* thisptr, void* a2) {
        auto screen_name = thisptr->getVisualTree()->getUIControl()->mName;
        static std::string cached_screen_name = "";

        selaura::sv_render_event ev{};
        ev.sv = thisptr;

        if (screen_name != "debug_screen" && screen_name != "toast_screen") {
            cached_screen_name = screen_name;
        }

        ev.screen_name = cached_screen_name;

        selaura::get<selaura::event_manager>().dispatch(ev);

        return selaura::hook<&ScreenView::render>::call(thisptr, a2);
    }
};