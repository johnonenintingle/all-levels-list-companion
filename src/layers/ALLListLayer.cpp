#include "ALLListLayer.hpp"
#include "ProScrollLayer.hpp"

#include "nodes/ALLLevelCell.hpp"

#include <Geode/ui/Button.hpp>

ALLListLayer* ALLListLayer::create() {
    auto ret = new ALLListLayer();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

CCScene* ALLListLayer::scene() {
    auto layer = ALLListLayer::create();
    auto scene = CCScene::create();
    scene->addChild(layer);
    return scene;
}

bool ALLListLayer::init() {
    CCLayer::init();

    this->setKeypadEnabled(true);
    this->addChild(createLayerBG());

    auto winSize = CCDirector::get()->getWinSize();

    auto btn = Button::createWithSpriteFrameName("GJ_arrow_03_001.png", [this](Button*) {
        this->keyBackClicked();
    });
    btn->setPosition(CCPoint{0, winSize.height} + CCPoint{btn->getScaledContentWidth() / 2.f + 8.f, -btn->getScaledContentHeight() / 2.f - 4.f});

    this->addChild(btn);

    auto logo = CCSprite::create("all_logo_full.png"_spr);
    logo->setScale(0.8785f);
    logo->setAnchorPoint({0.5f, 1});
    logo->setPosition({winSize.width / 2.f, winSize.height - 13.f});

    this->addChild(logo);

    auto bg = CCLayerColor::create({0, 0, 0, 67}, 362, 247);
    bg->setPosition(winSize / 2.f + CCPoint{0, -14});
    bg->ignoreAnchorPointForPosition(false);

    this->addChild(bg);

    auto scroll = ProScrollLayer::create(bg->getContentSize());
    scroll->setPosition(bg->getPosition());
    scroll->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(0.f));

    this->addChild(scroll);

    scroll->m_contentLayer->addChild(ALLLevelCell::create());

    scroll->m_contentLayer->updateLayout();
    scroll->scrollToTop();

    auto border = NineSlice::create("green-border.png"_spr);
    border->setPosition(winSize / 2.f + CCPoint{0, -14});
    border->setContentSize({368, 253});

    this->addChild(border);

    return true;
}

void ALLListLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}