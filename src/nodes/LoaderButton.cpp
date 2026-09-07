#include "LoaderButton.hpp"

#include "actions/Shake.hpp"

LoaderButton::LoaderButton(Function<void()> callback)
    : m_callback(std::move(callback)) {}

LoaderButton* LoaderButton::create(ZStringView file, float scale, CCPoint offset, Function<void()> callback) {
    auto ret = new LoaderButton(std::move(callback));

    if (ret->init(file, scale, offset)) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool LoaderButton::init(ZStringView file, float scale, CCPoint offset) {
    this->setAnchorPoint({0.5f, 0.5f});
    
    m_plainSprite = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    m_plainSprite->setAnchorPoint({0, 0});
    
    m_grayscalePlainSprite = CCSpriteGrayscale::createWithSpriteFrameName("GJ_plainBtn_001.png");
    m_grayscalePlainSprite->setAnchorPoint({0, 0});
    m_grayscalePlainSprite->setColor({ 90, 90, 90 });

    auto sprite = CCSprite::create(file.c_str());
    sprite->setScale(scale);
    sprite->setPosition(m_plainSprite->getContentSize() / 2.f + offset);

    m_plainSprite->addChild(sprite);

    m_grayscaleSprite = CCSpriteGrayscale::create(file.c_str());
    m_grayscaleSprite->setScale(scale);
    m_grayscaleSprite->setPosition(m_plainSprite->getContentSize() / 2.f + offset);
    m_grayscaleSprite->setColor({ 99, 99, 99 });
    m_grayscaleSprite->setOpacity(110);

    m_grayscalePlainSprite->addChild(m_grayscaleSprite);

    auto container = CCNode::create();
    container->setContentSize(m_plainSprite->getContentSize());

    container->addChild(m_plainSprite);
    container->addChild(m_grayscalePlainSprite);

    m_button = Button::createWithNode(container, [this](Button*) {
        if (!m_loading && !m_error) {
            m_callback();
        } else if (!m_errorString.empty()) {
            FLAlertLayer::create(
                "Error",
                m_errorString,
                "Ok"
            )->show();
        }
    });
    m_button->setScaleMultiplier(1.1f);
    m_button->setPosition(m_button->getContentSize() / 2.f);

    this->addChild(m_button);

    m_loadingCircle = CCSprite::create("loadingCircle.png");
    m_loadingCircle->setPosition(m_button->getContentSize() / 2.f);
    m_loadingCircle->setScale(0.365f);
    m_loadingCircle->setOpacity(0);
    m_loadingCircle->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    m_loadingCircle->runAction(CCEaseSineIn::create(CCFadeTo::create(0.09f, 180)));
    m_loadingCircle->runAction(CCRepeatForever::create(
        CCRotateBy::create(1.1f, 360)
    ));

    this->addChild(m_loadingCircle);

    this->setContentSize(m_button->getContentSize());
    this->setLoading(true);

    return true;
}

void LoaderButton::setLoading(bool loading) {
    m_loadingCircle->setVisible(loading);
    m_plainSprite->setVisible(!loading);
    m_grayscalePlainSprite->setVisible(loading);
    m_button->setEnabled(!loading);
 
    m_loading = loading;
}

void LoaderButton::setError(std::string error) {
    this->setLoading(true);

    m_button->setEnabled(!error.empty());
    m_button->setScaleMultiplier(1.f);

    m_errorString = std::move(error);

    m_loadingCircle->setVisible(false);

    m_grayscalePlainSprite->runAction(CCSpawn::create(
        Shake::create(0.25f, 0.52f),
        CCEaseSineOut::create(CCTintTo::create(1.35f, 90, 70, 70)),
        nullptr
    ));

    m_grayscaleSprite->runAction(CCEaseSineOut::create(CCTintTo::create(1.35f, 99, 73, 73)));

    m_error = true;
}