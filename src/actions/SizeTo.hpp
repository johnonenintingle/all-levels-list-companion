#pragma once

#include "Includes.hpp"

class SizeTo : public CCActionInterval {

private:

    CCSize m_startSize;
    CCSize m_targetSize;

    SizeTo(const CCSize& targetSize)
        : m_targetSize(targetSize) {}

public:

    void startWithTarget(CCNode* target) override {
        CCActionInterval::startWithTarget(target);
        
        if (target) {
            m_startSize = target->getContentSize();
        }
    }

    void update(float progress) override {
        if (m_pTarget) {
            float newWidth = m_startSize.width + (m_targetSize.width - m_startSize.width) * progress;
            float newHeight = m_startSize.height + (m_targetSize.height - m_startSize.height) * progress;
            m_pTarget->setContentSize({newWidth, newHeight});
        }
    }

    static SizeTo* create(float duration, const CCSize& targetSize) {
        auto ret = new SizeTo(targetSize);

        if (ret->initWithDuration(duration)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

};