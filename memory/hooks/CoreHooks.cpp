#include "CoreHooks.hpp"

void selaura::detour<&MinecraftGame::_update>::hk(MinecraftGame* thisptr) {
    return selaura::hook<&MinecraftGame::_update>::call(thisptr);
}