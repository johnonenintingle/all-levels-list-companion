#include "DifficultyNode.hpp"

#include "actions/SizeTo.hpp"

DifficultyNode::DifficultyNode(Difficulty difficulty)
    : m_difficulty(difficulty) {}

DifficultyNode* DifficultyNode::create(Difficulty difficulty) {
    auto ret = new DifficultyNode(difficulty);

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool DifficultyNode::init() {
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({55, 43});

    m_bg = NineSlice::create("square02b_001.png");
    m_bg->setColor({0, 0, 0});
    m_bg->setOpacity(50);
    m_bg->setPosition(this->getContentSize() / 2.f);
    m_bg->setContentSize(this->getContentSize());

    this->addChild(m_bg);
    
    auto stencil = NineSlice::create("square02b_001.png");
    stencil->setContentSize(this->getContentSize());
    stencil->setPosition(this->getContentSize() / 2.f);

    auto clip = CCClippingNode::create();
    clip->setAlphaThreshold(0.01f);
    clip->setStencil(stencil);

    this->addChild(clip);

    auto color = colorForDifficulty(m_difficulty);

    m_gradient = CCLayerGradient::create({color.r, color.g, color.b, 27}, {color.r, color.g, color.b, 0}, {1, 0});
    m_gradient->setContentSize({27, this->getContentHeight()});

    clip->addChild(m_gradient);

    CCSprite* spr = nullptr;

    if (static_cast<int>(m_difficulty) <= 5) {
        spr = CCSprite::createWithSpriteFrameName(
            m_difficulty == Difficulty::Auto
                ? "difficulty_auto_btn_001.png"
                : fmt::format("difficulty_{:02}_btn_001.png", static_cast<int>(m_difficulty)).c_str()
        );
        spr->setAnchorPoint({0, 0.5f});
        spr->setPosition({5, this->getContentHeight() / 2.f + 1.2f});
    } else if (m_difficulty == Difficulty::Impossible) {
        spr = CCSprite::create("impossible_demon.png"_spr);
        spr->setAnchorPoint({0, 1});
        spr->setPosition({5, 40});
    } else {
        spr = CCSprite::createWithSpriteFrameName(fmt::format("difficulty_{:02}_btn2_001.png", static_cast<int>(m_difficulty)).c_str());
        spr->setAnchorPoint({0, 1});
        spr->setPosition({5, 40});
    }

    spr->setScale(0.5f);

    this->addChild(spr);

    m_countLabel = CCLabelBMFont::create("-", "bigFont.fnt");
    m_countLabel->setOpacity(140);
    m_countLabel->setScale(0.45f);
    m_countLabel->setPosition({
        (this->getContentWidth() - (spr->getPositionX() + spr->getScaledContentWidth())) / 2.f + spr->getPositionX() + spr->getScaledContentWidth() + 0.2f, 
        this->getContentHeight() / 2.f + 1.2f
    });

    this->addChild(m_countLabel);
    
    return true;
}

Difficulty DifficultyNode::getDifficulty() {
    return m_difficulty;
}

void DifficultyNode::setCount(int count) {
    m_countLabel->stopAllActions();
    m_countLabel->runAction(CCSequence::create(
        CCFadeTo::create(0.07f, 90),
        CallFuncExt::create([this, count] {
            m_countLabel->setString(numToString(count).c_str());
            m_countLabel->limitLabelWidth(18.225f, 0.45f, 0.f);
        }),
        CCFadeTo::create(0.07f, 140),
        nullptr
    ));
}

void DifficultyNode::setHovered(bool hovered) {
    if (hovered == m_hovered) {
        return;
    }

    m_hovered = hovered;

    this->stopAllActions();
    this->runAction(CCEaseSineInOut::create(CCScaleTo::create(0.17f, hovered ? 1.02f : 1.f)));

    auto color = hovered ? ccColor3B{25, 25, 25} : ccColor3B{0, 0, 0};

    m_bg->stopAllActions();
    m_bg->runAction(CCEaseSineInOut::create(CCTintTo::create(0.17f, color.r, color.g, color.b)));

    m_gradient->stopAllActions();
    m_gradient->runAction(CCEaseSineInOut::create(SizeTo::create(0.17f, {hovered ? this->getContentWidth() + 10.f : 27, m_gradient->getContentHeight()})));
}