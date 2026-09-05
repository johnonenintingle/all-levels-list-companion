#pragma once

#include "Includes.hpp"

class RatingSlider : public CCLayer {

private:

    Function<void(float)> m_callback;

    CCLayerColor* m_stencil = nullptr;
    CCNode* m_hitbox = nullptr;
    CCSprite* m_thumb = nullptr;

    float m_value = 0.f;
    float m_targetValue = 0.f;

    RatingSlider(Function<void(float)>);

    bool init() override;

    void setValueForPosition(const CCPoint&);
    void setHeld(bool);

    bool ccTouchBegan(CCTouch*, CCEvent*) override;
    void ccTouchMoved(CCTouch*, CCEvent*) override;
    void ccTouchEnded(CCTouch*, CCEvent*) override;

    void update(float) override;

public:

    static RatingSlider* create(Function<void(float)>);

    void setValue(float);
    float getValue();

};