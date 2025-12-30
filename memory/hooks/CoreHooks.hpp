#pragma once
#include <pch.hpp>

template <>
struct selaura::detour<&MinecraftGame::_update> {
    static void hk(MinecraftGame* thisptr);
};