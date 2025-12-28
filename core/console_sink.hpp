#pragma once
#include <pch.hpp>

namespace selaura {
    class console_sink final : public spdlog::sinks::base_sink<std::mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t buf;
            formatter_->format(msg, buf);

            ImVec4 log_color = level_to_color(msg.level);
            selaura::console->push_text(fmt::to_string(buf), log_color);
        }

        void flush_() override {}
    private:
        ImVec4 level_to_color(spdlog::level::level_enum level) {
            switch (level) {
            case spdlog::level::trace:    return ImVec4(0.745f, 0.584f, 1.000f, 1.00f);
            case spdlog::level::debug:    return ImVec4(0.471f, 0.663f, 1.000f, 1.00f);
            case spdlog::level::info:     return ImVec4(0.949f, 0.957f, 0.973f, 1.00f);
            case spdlog::level::warn:     return ImVec4(0.933f, 0.325f, 0.588f, 1.00f);
            case spdlog::level::err:      return ImVec4(1.000f, 0.494f, 0.714f, 1.00f);
            case spdlog::level::critical: return ImVec4(0.933f, 0.325f, 0.588f, 1.00f);
            default:                      return ImVec4(0.949f, 0.957f, 0.973f, 1.00f);
            }
        }
    };
};