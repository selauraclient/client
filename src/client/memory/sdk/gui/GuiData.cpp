#include "GuiData.hpp"
#include "../../patcher.hpp"

void GuiData::displayClientMessage_hk(const std::string& message) {
	selaura::call_fn<&GuiData::displayClientMessage_hk>(this, message);
}

