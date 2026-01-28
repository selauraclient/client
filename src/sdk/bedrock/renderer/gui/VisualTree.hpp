#pragma once
#include <pch.hpp>
#include "UIControl.hpp"

class VisualTree {
public:
    UIControl* getUIControl() {
        return hat::member_at<UIControl*>(this, 0x8);
    }
};