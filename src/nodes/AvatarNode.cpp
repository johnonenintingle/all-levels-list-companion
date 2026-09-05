#include "AvatarNode.hpp"

AvatarNode* AvatarNode::create(const std::string& url) {
    auto ret = new AvatarNode();

    if (ret->init(url)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool AvatarNode::init(const std::string& url) {
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({24.875f, 24.875f});

    auto fg = NineSlice::create("geode.loader/slider-groove-2.png");
    fg->setScale(0.25f);
    fg->setContentSize(this->getContentSize() / fg->getScale());
    fg->setPosition(this->getContentSize() / 2.f);

    this->addChild(fg, 1);
    
    auto bg = NineSlice::create("square02b_001.png");
    bg->setScale(fg->getScale());
    bg->setContentSize(this->getContentSize() / fg->getScale());
    bg->setColor({0, 0, 0});
    bg->setOpacity(90);
    bg->setPosition(this->getContentSize() / 2.f);

    this->addChild(bg);

    auto stencil = NineSlice::create("square02b_001.png");
    stencil->setScale(fg->getScale());
    stencil->setContentSize(this->getContentSize() / fg->getScale());
    stencil->setPosition(this->getContentSize() / 2.f);

    auto clip = CCClippingNode::create();
    clip->setAlphaThreshold(0.01f);
    clip->setStencil(stencil);

    this->addChild(clip);

    auto loadingCircle = CCSprite::create("loadingCircle.png");
    loadingCircle->setPosition(this->getContentSize() / 2.f);
    loadingCircle->setScale(0.225f);
    loadingCircle->setOpacity(178);
    loadingCircle->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    loadingCircle->runAction(CCRepeatForever::create(
        CCRotateBy::create(1.1f, 360)
    ));

    this->addChild(loadingCircle);

    auto sprite = LazySprite::create(this->getContentSize(), false);
    sprite->setPosition(this->getContentSize() / 2.f);
    sprite->loadFromUrl(url, LazySprite::Format::kFmtWebp, false);
    sprite->setLoadCallback([size = this->getContentSize(), selfref = WeakRef(this), loadingCircle, sprite](Result<> res) {
        auto self = selfref.lock();

        if (!self) {
            return;
        }

        if (res.isOk()) {
            sprite->setScale(std::min(size.width / sprite->getContentWidth(), size.height / sprite->getContentHeight()));
        } else {
            auto lbl = CCLabelBMFont::create("N/A", "bigFont.fnt");
            lbl->setPosition(self->getContentSize() / 2.f + CCPoint{0.5f, 0.5f});
            lbl->setOpacity(89);
            lbl->setScale(0.29f);

            self->addChild(lbl);
        }

        loadingCircle->setVisible(false);
    });
    sprite->setScale(std::min(this->getContentWidth() / sprite->getContentWidth(), this->getContentHeight() / sprite->getContentHeight()));

    clip->addChild(sprite);

    if (sprite->isLoaded()) {
        loadingCircle->setVisible(false);
    }

    return true;
}