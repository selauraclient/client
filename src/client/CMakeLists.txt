cmake_minimum_required(VERSION 3.28)
project(selaura_client LANGUAGES CXX)

add_library(selaura_client SHARED
    main.cpp
    client.cpp
    platform/platform.cpp
    memory/sdk/gui/ScreenView.cpp
    memory/sdk/world/BaseLightTextureImageBuilder.cpp
    memory/sdk/world/Dimension.cpp
    memory/sdk/game/uri/GameArguments.cpp
    memory/sdk/renderer/screen/MinecraftUIRenderContext.cpp
    memory/sdk/renderer/bgfx/bgfx.cpp
    memory/sdk/core/RenderItemInHandDescription.cpp
    memory/sdk/client/ClientInstanceScreenModel.cpp
    memory/sdk/gui/GuiData.cpp
    memory/sdk/game/network/MinecraftPackets.cpp
    feature/impl/render/fullbright.cpp
    feature/impl/render/paperdoll.cpp
    feature/impl/render/enchant_glint.cpp
    feature/impl/render/environment.cpp
)

target_include_directories(selaura_client PRIVATE
    fmt
    EnTT
    spdlog
    libhat
    gsl
    glm
    safetyhook
    glaze
    magic_enum
)

target_link_libraries(selaura_client PRIVATE
    EnTT::EnTT
    spdlog
    libhat
    Microsoft.GSL::GSL
    glm
    safetyhook::safetyhook
    glaze::glaze
    magic_enum::magic_enum
)