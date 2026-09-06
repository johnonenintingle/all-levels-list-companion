#include "LoadingScreenLayer.hpp"

LoadingScreenLayer::~LoadingScreenLayer() {
    queueInMainThread([self = Ref(this)] {
        CCTouchDispatcher::get()->removeDelegate(self.data());
    });
}

LoadingScreenLayer::LoadingScreenLayer(CCNode* closeBtn)
    : m_closeBtn(closeBtn) {}

LoadingScreenLayer* LoadingScreenLayer::create(ZStringView text, const CCSize& size, CCNode* closeBtn) {
    auto ret = new LoadingScreenLayer(closeBtn);

    if (ret->init(text, size)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool LoadingScreenLayer::init(ZStringView text, const CCSize& size) {
    CCLayer::init();

    this->setContentSize(size);
    this->setTouchEnabled(true);

    auto bg = NineSlice::create("GJ_square01.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(0);
    bg->setAnchorPoint({0, 0});
    bg->setContentSize(size);
    bg->runAction(CCEaseSineIn::create(CCFadeTo::create(0.09f, 200)));

    this->addChild(bg);

    auto loadingCircle = CCSprite::create("loadingCircle.png");
    loadingCircle->setPosition(size / 2.f + CCPoint{0, 20});
    loadingCircle->setScale(0.725f);
    loadingCircle->setOpacity(0);
    loadingCircle->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    loadingCircle->runAction(CCEaseSineIn::create(CCFadeTo::create(0.09f, 178)));
    loadingCircle->runAction(CCRepeatForever::create(
        CCRotateBy::create(1.1f, 360)
    ));

    this->addChild(loadingCircle);

    m_label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    m_label->setScale(0.425f);
    m_label->setPosition(size / 2.f + CCPoint{0, -20});
    m_label->setOpacity(0);
    m_label->runAction(CCEaseSineIn::create(CCFadeTo::create(0.09f, 170)));

    this->addChild(m_label);

    return true;
}

void LoadingScreenLayer::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -700, true);
}

bool LoadingScreenLayer::ccTouchBegan(CCTouch* touch, CCEvent*) {
    return this->isVisible() && isHoveringNode(touch->getLocation(), this) && !isHoveringNode(touch->getLocation(), m_closeBtn);
}

void LoadingScreenLayer::setLoadingText(ZStringView text) {
    m_label->setString(text.c_str());
}