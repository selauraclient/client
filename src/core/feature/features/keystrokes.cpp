#include "keystrokes.hpp"

#include "core/service_manager.hpp"
#include "core/event/event_manager.hpp"
#include "core/renderer/sgfx.hpp"
#include "sdk/bedrock/actor/components/MoveInputComponent.hpp"

namespace selaura {
    void keystrokes::on_enable() {
        selaura::get<selaura::event_manager>().subscribe(this, &keystrokes::on_render);
        selaura::get<selaura::event_manager>().subscribe(this, &keystrokes::on_mcupdate);
    }

    void keystrokes::on_disable() {
        selaura::get<selaura::event_manager>().unsubscribe(this, &keystrokes::on_render);
        selaura::get<selaura::event_manager>().unsubscribe(this, &keystrokes::on_mcupdate);
    }

    void keystrokes::on_render(render_event& ev) {
        auto approach = [](float cur, float target, float step) {
            if (cur < target) return std::min(cur + step, target);
            if (cur > target) return std::max(cur - step, target);
            return cur;
        };

        constexpr float step = 1.f / 12.f;

        up_t = approach(up_t,    up ? 1.f : 0.f, step);
        left_t = approach(left_t,  left ? 1.f : 0.f, step);
        right_t = approach(right_t, right ? 1.f : 0.f, step);
        down_t = approach(down_t,  down ? 1.f : 0.f, step);
        jump_t = approach(jump_t,  jump ? 1.f : 0.f, step);

        auto box = [&](float x, float y, float w, float h, float t) {
            sgfx::draw_rect(x, y, w, h, {t, t, t, 0.45f}, {5, 5, 5, 5});
        };

        float key = 40.f;
        float gap = 4.f;
        float spaceH = key * 0.6f;

        float totalW = key * 3 + gap * 2;
        float totalH = key * 2 + gap * 2 + spaceH;

        float x = ev.screen_width - totalW - 20.f;
        float y = ev.screen_height - totalH - 20.f;

        box(x + key + gap, y, key, key, up_t);
        box(x,y + key + gap, key, key, left_t);
        box(x + key + gap, y + key + gap, key, key, down_t);
        box(x + (key + gap) * 2.f, y + key + gap, key, key, right_t);
        box(x, y + key * 2 + gap * 2, totalW, spaceH, jump_t);
    }


    void keystrokes::on_mcupdate(mcgame_update& ev) {
        if (LocalPlayer* lp = ev.mc->getPrimaryLocalPlayer(); lp) {
            auto& ent = lp->getEntityContext();

            if (auto* mvc = ent.tryGetComponent<MoveInputComponent>(); mvc) {
                auto flags = mvc->mRawInputState.mFlagValues;
                this->up = flags.test((size_t)MoveInputState::Flag::Up);
                this->left = flags.test((size_t)MoveInputState::Flag::Left);
                this->right = flags.test((size_t)MoveInputState::Flag::Right);
                this->down = flags.test((size_t)MoveInputState::Flag::Down);
                this->jump = flags.test((size_t)MoveInputState::Flag::JumpDown);
            }
        }
    }
};