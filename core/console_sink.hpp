#pragma once
#include <pch.hpp>

namespace selaura {
    class console_sink final : public spdlog::sinks::base_sink<std::mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t buf;
            formatter_->format(msg, buf);
            selaura::console->push_text(fmt::to_string(buf));
        }

        void flush_() override {}
    };
};