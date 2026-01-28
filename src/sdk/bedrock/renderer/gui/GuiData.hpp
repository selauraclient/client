#pragma once
#include <pch.hpp>

struct GuiData {
    MCAPI void displayClientMessage(
        const std::string& message,
        const std::optional<std::string>& filteredMessage = {},
        bool ttsRequired = false
    );
};