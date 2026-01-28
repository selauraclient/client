#include "coords.hpp"

#include "core/service_manager.hpp"
#include "core/event/event_manager.hpp"
#include "core/renderer/sgfx.hpp"
#include "sdk/bedrock/actor/components/StateVectorComponent.hpp"
#include "sdk/bedrock/actor/components/OffsetsComponent.hpp"

#include "sdk/core/actor/actor.hpp"

namespace selaura {
    void coords::on_enable() {
        selaura::get<selaura::event_manager>().subscribe(this, &coords::on_render);
        selaura::get<selaura::event_manager>().subscribe(this, &coords::on_mcupdate);
    }

    void coords::on_disable() {
        selaura::get<selaura::event_manager>().unsubscribe(this, &coords::on_render);
        selaura::get<selaura::event_manager>().unsubscribe(this, &coords::on_mcupdate);
    }

    void coords::on_render(render_event& ev) {
        auto str = fmt::format("{}, {}, {}", std::floor(this->pos.x), std::floor(this->pos.y), std::floor(this->pos.z));
        sgfx::draw_text(str, 10, 10, 24.f);
    }

    void coords::on_mcupdate(mcgame_update& ev) {
        using namespace selaura::sdk;
        if (LocalPlayer* lp = ev.mc->getPrimaryLocalPlayer(); lp) {
            auto act = std::make_shared<selaura_actor>(lp);
            this->pos = act->get_position(true);
        }
    }
};