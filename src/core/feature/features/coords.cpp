#include "coords.hpp"

#include "core/service_manager.hpp"
#include "core/event/event_manager.hpp"
#include "core/renderer/sgfx.hpp"
#include "sdk/actor/components/StateVectorComponent.hpp"

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
        sgfx::draw_text(fmt::format("Position: {}, {}, {}", (int)this->pos.x, (int)this->pos.y, (int)this->pos.z), 10, 10, 32.f);
    }

    void coords::on_mcupdate(mcgame_update& ev) {
        if (LocalPlayer* lp = ev.mc->getPrimaryLocalPlayer(); lp) {
            auto& ent = lp->getEntityContext();

            if (auto* cmp = ent.tryGetComponent<StateVectorComponent>(); cmp) {
                this->pos = cmp->mPos;
            }
        }
    }
};