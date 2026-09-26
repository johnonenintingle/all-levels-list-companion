#include "ALLLevelCell.hpp"

ALLLevelCell* ALLLevelCell::create() {
    auto ret = new ALLLevelCell();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool ALLLevelCell::init() {
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({362, 65});

    this->addChild(
        CCLayerColor::create({194, 114, 62, 255}, this->getContentWidth(), this->getContentHeight())
    );



    return true;
}