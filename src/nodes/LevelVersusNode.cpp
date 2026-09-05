#include "LevelVersusNode.hpp"

#include "actions/SizeTo.hpp"

LevelVersusNode* LevelVersusNode::create() {
    auto ret = new LevelVersusNode();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

LevelVersusNode* LevelVersusNode::create(const LevelRanking& level) {
    auto ret = new LevelVersusNode();

    if (ret->init(level)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool LevelVersusNode::init() {
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({95, 73});

    m_overlay = CCLayerColor::create({137, 76, 46, 0}, this->getContentWidth(), this->getContentHeight());

    this->addChild(m_overlay, 12);

    m_bg = NineSlice::create("square02b_001.png");
    m_bg->setColor({0, 0, 0});
    m_bg->setOpacity(50);
    m_bg->setContentSize(this->getContentSize());
    m_bg->setPosition(this->getContentSize() / 2.f);

    this->addChild(m_bg);

    auto stencil = NineSlice::create("square02b_001.png");
    stencil->setContentSize(this->getContentSize());
    stencil->setPosition(this->getContentSize() / 2.f);

    auto clip = CCClippingNode::create();
    clip->setAlphaThreshold(0.01f);
    clip->setStencil(stencil);

    this->addChild(clip);

    m_gradient = CCLayerGradient::create({100, 100, 100, 70}, {100, 100, 100, 0});
    m_gradient->setContentSize(this->getContentSize() - CCSize{0, 25});
    m_gradient->setPositionY(this->getContentHeight());
    m_gradient->setAnchorPoint({0, 1});
    m_gradient->ignoreAnchorPointForPosition(false);

    clip->addChild(m_gradient);

    m_hoverGradient = CCLayerGradient::create({255, 0, 0, 50}, {255, 0, 0, 0});
    m_hoverGradient->setContentSize({this->getContentWidth(), 0});
    m_hoverGradient->setScaleY(-1);

    clip->addChild(m_hoverGradient);

    m_border = NineSlice::create("border.png"_spr);
    m_border->setContentSize(this->getContentSize());
    m_border->setPosition(this->getContentSize() / 2.f);

    this->addChild(m_border, 1);

    m_sprite = CCSprite::createWithSpriteFrameName("diffIcon_00_btn_001.png");
    m_sprite->setScale(0.8f);
    m_sprite->setPosition(this->getContentSize() / 2.f + CCPoint{0, 11});

    this->addChild(m_sprite);
    
    m_nameLabel = CCLabelBMFont::create("-", "bigFont.fnt");
    m_nameLabel->setPosition({this->getContentWidth() / 2.f, 20});
    m_nameLabel->limitLabelWidth(76.59f, 0.345f, 0.f);
    m_nameLabel->setOpacity(215);

    m_ogLabelScale = m_nameLabel->getScale();

    this->addChild(m_nameLabel);

    return true;
}


bool LevelVersusNode::init(const LevelRanking& level) {
    LevelVersusNode::init();
    
    this->setLevel(level);

    return true;
}

void LevelVersusNode::setLevel(const LevelRanking& level, bool special, bool special2) {
    if (special) {
        m_animating = true;

        if (special2) {
            if (m_sprite) {
                m_sprite->removeFromParent();
            }

            m_sprite = iconForDifficulty(level.bucket);
            m_sprite->setOpacity(0);
            m_sprite->runAction(CCFadeTo::create(0.1f, 255));
            m_sprite->setScale(0.8f);
            m_sprite->setPosition(this->getContentSize() / 2.f + CCPoint{0, 11});

            this->addChild(m_sprite);

            auto color = colorForDifficulty(level.bucket);

            m_gradient->setStartColor(color);
            m_gradient->setEndColor(color);
        }

        m_nameLabel->runAction(CCFadeTo::create(0.08f, 0));
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.081f),
            CallFuncExt::create([this, level] {
                m_nameLabel->setString(level.name.c_str());
                
                auto prevScale = m_nameLabel->getScale();

                m_nameLabel->limitLabelWidth(76.59f, 0.345f, 0.f);
                m_ogLabelScale = m_nameLabel->getScale();
                m_nameLabel->setScale(prevScale);
                m_nameLabel->stopAllActions();

                if (m_mode == Mode::Main) {
                    m_nameLabel->runAction(CCSpawn::create(
                        CCEaseSineInOut::create(CCMoveTo::create(0.1f, {m_nameLabel->getPositionX(), 20.f})),
                        CCEaseSineInOut::create(CCScaleTo::create(0.1f, m_ogLabelScale)),
                        CCFadeTo::create(0.15f, 215),
                        nullptr
                    ));
                } else {
                    m_nameLabel->runAction(CCSpawn::create(
                        CCEaseSineInOut::create(CCMoveTo::create(0.1f, {m_nameLabel->getPositionX(), m_mode == Mode::Top ? 65.5f : 9.5f})),
                        CCEaseSineInOut::create(CCScaleTo::create(0.1f, m_ogLabelScale * 0.71f)),
                        CCFadeTo::create(0.15f, 215),
                        nullptr
                    ));
                }
            }),
            CCDelayTime::create(0.11f),
            CallFuncExt::create([this] {
                m_animating = false;
            }),
            nullptr
        ));

        return;
    }

    if (m_sprite) {
        m_sprite->removeFromParent();
    }

    m_sprite = iconForDifficulty(level.bucket);
    m_sprite->setScale(0.8f);
    m_sprite->setPosition(this->getContentSize() / 2.f + CCPoint{0, 11});

    this->addChild(m_sprite);
    
    m_nameLabel->setString(level.name.c_str());
    m_nameLabel->limitLabelWidth(76.59f, 0.345f, 0.f);

    m_ogLabelScale = m_nameLabel->getScale();

    auto color = colorForDifficulty(level.bucket);

    m_gradient->setStartColor(color);
    m_gradient->setEndColor(color);
}

void LevelVersusNode::setHovering(bool hovering) {
    if (m_animating) {
        return;
    }

    if (m_mode != Mode::Main) {
        hovering = false;
    }

    if (m_hovering == hovering) {
        return;
    }

    m_hovering = hovering;

    this->stopActionByTag(177);

    auto action = CCEaseSineInOut::create(CCScaleTo::create(0.17f, hovering ? 1.02f : 1.f));
    action->setTag(177);

    this->runAction(action);

    m_sprite->stopAllActions();
    m_sprite->runAction(CCEaseSineInOut::create(CCScaleTo::create(0.17f, hovering ? 0.809f : 0.8f)));

    auto color = hovering ? ccColor3B{90, 90, 90} : ccColor3B{0, 0, 0};

    m_bg->stopAllActions();
    m_bg->runAction(CCEaseSineInOut::create(CCTintTo::create(0.17f, color.r, color.g, color.b)));
    
    m_hoverGradient->stopAllActions();
    m_hoverGradient->runAction(CCEaseSineInOut::create(SizeTo::create(0.17f, {this->getContentWidth(), hovering ? 24.f : 0.f})));
 
    color = hovering ? ccColor3B{252, 255, 224} : ccColor3B{255, 255, 255};

    m_border->stopAllActions();
    m_border->runAction(CCEaseSineInOut::create(CCTintTo::create(0.17f, color.r, color.g, color.b)));

    m_nameLabel->stopActionByTag(178);
    
    action = CCEaseSineInOut::create(CCFadeTo::create(0.17f, hovering ? 255 : 215));
    action->setTag(178);

    m_nameLabel->runAction(action);
}

void LevelVersusNode::setMode(Mode mode, bool instant) {
    m_mode = mode;
    
    if (instant) {
        switch (mode) {
            default: {
                m_gradient->setContentSize({m_gradient->getContentWidth(), 0});
                m_nameLabel->setPosition({m_nameLabel->getPositionX(), mode == Mode::Top ? 65.5f : 9.5f});
                m_nameLabel->setScale(m_ogLabelScale * 0.71f);
                m_sprite->setOpacity(0);
                break;
            }
            case Mode::Main: {
                m_gradient->setContentSize({m_gradient->getContentWidth(), this->getContentHeight() - 25});
                m_nameLabel->setPosition({this->getContentWidth() / 2.f, 20});
                m_nameLabel->setScale(m_ogLabelScale);
                m_sprite->setOpacity(255);
                break;
            }
        }

        return;
    }

    switch (mode) {
        default: {
            m_gradient->runAction(CCEaseSineInOut::create(SizeTo::create(0.2f, {m_gradient->getContentWidth(), 0})));
            m_nameLabel->runAction(CCSpawn::create(
                CCEaseSineInOut::create(CCMoveTo::create(0.2f, {m_nameLabel->getPositionX(), mode == Mode::Top ? 65.5f : 9.5f})),
                CCEaseSineInOut::create(CCScaleTo::create(0.2f, m_ogLabelScale * 0.71f)),
                nullptr
            ));
            m_sprite->runAction(CCEaseSineInOut::create(CCFadeTo::create(0.2f, 0)));
            break;
        }
        case Mode::Main: {
            m_gradient->runAction(CCEaseSineInOut::create(SizeTo::create(0.2f, {m_gradient->getContentWidth(), this->getContentHeight() - 25})));
            m_nameLabel->runAction(CCSpawn::create(
                CCEaseSineInOut::create(CCMoveTo::create(0.2f, {this->getContentWidth() / 2.f, 20})),
                CCEaseSineInOut::create(CCScaleTo::create(0.2f, m_ogLabelScale)),
                nullptr
            ));
            m_sprite->runAction(CCEaseSineInOut::create(CCFadeTo::create(0.2f, 255)));
            break;
        }
    }
}

void LevelVersusNode::setHidden(bool hidden, bool instant) {
    if (instant) {
        m_overlay->setOpacity(hidden ? 255 : 0);
    } else {
        m_overlay->runAction(CCEaseSineInOut::create(CCFadeTo::create(0.18f, hidden ? 255 : 0)));
    }
}