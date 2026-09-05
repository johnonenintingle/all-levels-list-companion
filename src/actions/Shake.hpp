#pragma once

#include "Includes.hpp"

class Shake : public CCActionInterval {

private:

    CCPoint m_initialPosition = {0, 0};

    float m_strength = 0.f;

    Shake(float strength)
        : m_strength(strength) {}

    void startWithTarget(CCNode* target) override {
        CCActionInterval::startWithTarget(target);
        m_initialPosition = target->getPosition();
    }

    void update(float time) override {
        float randX = (CCRANDOM_0_1() * 2 - 1) * m_strength;
        float randY = (CCRANDOM_0_1() * 2 - 1) * m_strength;
        m_pTarget->setPosition(m_initialPosition + ccp(randX, randY));
    }

public:

    static Shake* create(float duration, float strength) {
        auto ret = new Shake(strength);

        if (ret->initWithDuration(duration)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

};