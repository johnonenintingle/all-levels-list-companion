#include "RatingSlider.hpp"

#include "actions/SizeTo.hpp"

RatingSlider::RatingSlider(Function<void(float)> callback)
    : m_callback(std::move(callback)) {}

RatingSlider* RatingSlider::create(Function<void(float)> callback) {
    auto ret = new RatingSlider(std::move(callback));

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool RatingSlider::init() {
    CCLayer::init();
    
    this->setTouchEnabled(true);
    this->registerWithTouchDispatcher();
    this->setTouchMode(kCCTouchesOneByOne);
    this->setContentSize({111, 13});
    this->setAnchorPoint({0.5f, 0.5f});
    this->ignoreAnchorPointForPosition(false);

    auto groove = NineSlice::create("geode.loader/slider-groove-2.png");
    groove->setPosition(this->getContentSize() / 2.f);
    groove->setScale(0.68f);
    groove->setContentSize(this->getContentSize() / groove->getScale());

    this->addChild(groove, 1);

    auto bg = CCLayerColor::create({ 0, 0, 0, 20 });
    bg->setContentSize(this->getContentSize() - CCSize{2, 3});
    bg->setPosition(this->getContentSize() / 2.f);
    bg->ignoreAnchorPointForPosition(false);

    this->addChild(bg);

    m_stencil = CCLayerColor::create({255, 255, 255, 255}, 0, this->getContentHeight());
    m_stencil->ignoreAnchorPointForPosition(false);
    m_stencil->setAnchorPoint({0, 0.5f});
    m_stencil->setPositionY(this->getContentHeight() / 2.f);

    auto clip = CCClippingNode::create();
    clip->setStencil(m_stencil);

    this->addChild(clip);

    auto gradient = CCLayerGradient::create({ 155, 226, 248, 255 }, { 33, 186, 233, 255 }, {1, 0});
    gradient->setContentSize(this->getContentSize() - CCSize{2, 3});
    gradient->setPosition(this->getContentSize() / 2.f);
    gradient->ignoreAnchorPointForPosition(false);

    clip->addChild(gradient);

    auto container = CCNode::create();
    container->setAnchorPoint({0.5f, 0.5f});
    container->setPosition(this->getContentSize() / 2.f);
    container->setLayout(
        SimpleAxisLayout::create(Axis::Row)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setGap(10.f)
    );

    this->addChild(container);

    for (int i = 0; i < 9; i++) {
        container->addChild(
            CCLayerColor::create({255, 255, 255, 15}, 0.925f, 5.455f)
        );
    }

    container->updateLayout();

    m_thumb = CCSprite::create("thumb.png"_spr);
    m_thumb->setScale(0.85f);
    m_thumb->setPosition({2.5f, this->getContentHeight() / 2.f});

    this->addChild(m_thumb, 1);

    m_hitbox = CCNode::create();
    m_hitbox->setContentSize({this->getContentWidth() - 5.f, this->getContentHeight()});
    m_hitbox->setPositionX(2.5f);

    this->addChild(m_hitbox);

    this->scheduleUpdate();
    this->setValue(0.f);

    return true;
}

void RatingSlider::setHeld(bool held) {
    m_thumb->stopActionByTag(123);

    auto color = held ? ccColor3B{220, 220, 220} : ccColor3B{255, 255, 255};
    auto action = CCEaseSineInOut::create(CCTintTo::create(0.19f, color.r, color.g, color.b));
    action->setTag(123);

    m_thumb->runAction(action);
}

void RatingSlider::setValueForPosition(const CCPoint& pos) {
    this->setValue(
        m_hitbox->convertToNodeSpace(pos).x / m_hitbox->getContentWidth()
    );

    m_callback(this->getValue());
}

bool RatingSlider::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (isHoveringNode(touch->getLocation(), this)) {
        this->setValueForPosition(touch->getLocation());
        this->setHeld(true);
        return true;
    }
     
    return false;
}

void RatingSlider::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    this->setValueForPosition(touch->getLocation());
}

void RatingSlider::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    this->setHeld(false);
}

void RatingSlider::update(float dt) {
    if (std::abs(m_targetValue - m_value) < 0.001f) {
        m_value = m_targetValue;
        return;
    }

    m_value += (m_targetValue - m_value) * 30.f * dt;

    auto width = m_hitbox->getContentWidth() * m_value + 2.5f;
    
    m_thumb->setPositionX(width);
    m_stencil->setContentWidth(width);
}

void RatingSlider::setValue(float value) {
    m_targetValue = std::round(std::clamp(value, 0.f, 1.f) / 0.005f) * 0.005f;
}

float RatingSlider::getValue() {
    return m_targetValue;
}