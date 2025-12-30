#pragma once
#include <pch.hpp>

enum class SubClientId : unsigned char {
    Server = 0,
    PrimaryClient = 0,
    Client2 = 1,
    Client3 = 2,
    Client4 = 3,
    ExtraIdSlotStart = 100,
    EditorUI = 101,
};