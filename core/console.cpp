#include "console.hpp"

void selaura::console_impl::render() {
    ImGui::SetNextWindowSize(ImVec2(750.0f, 450.0f), ImGuiCond_Once);

    if (!ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoNav)) {
        ImGui::End();
        return;
    }

    ImGui::BeginChild(
        "ScrollingRegion",
        ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
        false,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    {
        std::lock_guard<std::mutex> lock(message_mutex);
        for (const auto& msg : message_history) {
            ImGui::TextUnformatted(msg.c_str());
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();

    ImGui::Separator();

    const float spacing = ImGui::GetStyle().ItemSpacing.x;

    float input_width =
        ImGui::GetContentRegionAvail().x;

    ImGui::SetNextItemWidth(input_width);

    bool submit = ImGui::InputText(
        "##ConsoleInput",
        input_buf,
        IM_ARRAYSIZE(input_buf),
        ImGuiInputTextFlags_EnterReturnsTrue
    );

    ImGui::SameLine();

    if (submit) {
        if (input_buf[0] != '\0') {
            push_text(input_buf);
            input_buf[0] = '\0';
        }
    }

    ImGui::End();

}

void selaura::console_impl::push_text(const std::string& text) {
    std::lock_guard<std::mutex> lock(message_mutex);
    this->message_history.push_back(text);
}

void selaura::console_impl::shutdown() {
    std::lock_guard<std::mutex> lock(message_mutex);
    message_history.clear();
    cmd_history.clear();
    input_buf[0] = '\0';
}
