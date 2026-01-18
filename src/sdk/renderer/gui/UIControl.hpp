#pragma once
#include <pch.hpp>
class UIControl : public std::enable_shared_from_this<UIControl> {
public:
    glm::vec2 mCachedPosition;
    bool mCachedPositionDirty : 1;
    bool mHover : 1;
    std::byte pad[2];
    bool mIsVisibleInTree : 1;
    bool mEnabled : 1;
    bool mAllAncestorsEnabled : 1;
    bool mSelected : 1;
    bool mClipsChildren : 1;
    bool mAllowClipping : 1;
    bool mIsClipped : 1;
    bool mEnableClippingScissorTest : 1;
    bool mIsTemplate : 1;
    bool mPropagateAlpha : 1;
    bool mDelayCollectionUpdate : 1;
    bool mTextToSpeechTriggered : 1;
    unsigned short mControlCollectionFlag;
    std::string mName;
    glm::vec2 mParentRelativePosition;
    glm::vec2 mSize;
    glm::vec2 mMinSize;
    glm::vec2 mMaxSize;
    float mAlpha;
    int mZOrder;
    int mLayer;
    glm::vec2 mClipOffset;
    unsigned int mClipChangeEventId;
};
