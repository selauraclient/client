#include "client.hpp"

#include "config/types.hpp"
#include "magic_enum/magic_enum_utility.hpp"
#include "memory/patcher.hpp"
#include "memory/sdk/game/network/PacketHandlerDispatcherInstance.hpp"

namespace selaura {
    void client::init() {
        try {
            auto start = std::chrono::steady_clock::now();

            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto logger = std::make_shared<spdlog::logger>("client_logger", sink);
            spdlog::set_default_logger(logger);

            spdlog::set_pattern("[%T] [client/%^%l%$] %v");
            spdlog::flush_on(spdlog::level::info);

            auto& feature_manager = this->get<selaura::feature_manager>();
            feature_manager.for_each([]<typename T>(std::shared_ptr<T>& f) {
                f->set_enabled(true);
                spdlog::info("Module Loaded: {}", selaura::feature_traits<T>::name.c_str());
            });

            selaura::patch_fns<
                &Dimension::Dimension_ctor_hk,
                &mce::framebuilder::RenderItemInHandDescription::RenderItemInHandDescription_ctor_hk,
                &ScreenView::setupAndRender_hk,
                &BaseLightTextureImageBuilder::createBaseLightTextureData_hk,
                &NetherLightTextureImageBuilder::createBaseLightTextureData_hk,
                &GameArguments::_onUri_hk,
                &Dimension::getTimeOfDay_hk,
                &bgfx::d3d11::RendererContextD3D11::submit_hk,
                &bgfx::d3d12::RendererContextD3D12::submit_hk,
                &ClientInstanceScreenModel::executeCommand_hk,
                &GuiData::displayClientMessage_hk
            >();

            magic_enum::enum_for_each<MinecraftPacketIds>([](auto val) {
                constexpr MinecraftPacketIds id = val;

                const auto pkt = selaura::call_original<&MinecraftPackets::createPacket_hk>(id);
                if (!pkt) return;

                Packet* packet = pkt.get();
                void* vtable = packet->handler;

                selaura::patch_vtable_fn<&PacketHandlerDispatcherInstance_callbacks<id>::handle>(vtable, 1);
            });

            auto end = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration<float, std::milli>(end - start).count();
            spdlog::info("Completed injection in {} ms.", static_cast<int>(ms));

            spdlog::info("Write \"help\" in the command line to see a list of commands.");

            //auto command_handler = this->get<selaura::command_handler>();
            //std::thread(&command_handler::start, command_handler).detach();
        } catch (const std::exception& e) {
            spdlog::info("std::exception: {}\n", e.what());
            this->unload();
        } catch (...) {
            spdlog::info("Unknown exception occurred.");
            this->unload();
        }
    }

    void client::unload() {
    }
};