#include "UnderlinedLabel.hpp"

UnderlinedLabel* UnderlinedLabel::create(ZStringView text, ZStringView font) {
    auto ret = new UnderlinedLabel();

    if (ret->init(text, font)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool UnderlinedLabel::init(ZStringView text, ZStringView font) {
    m_label = CCLabelBMFont::create(text.c_str(), font.c_str());
    m_label->setAnchorPoint({0, 0});

    this->addChild(m_label);

    this->setContentSize(m_label->getContentSize());
    this->setAnchorPoint({0.5f, 0.5f});

    m_underline = NineSlice::create("square02b_001.png");
    m_underline->setScale(0.12f);
    m_underline->setContentSize(CCSize{this->getContentWidth() + 1.f, 2.2f} / m_underline->getScale());
    m_underline->setPosition({this->getContentWidth() / 2.f, -1.4f});
    m_underline->setAnchorPoint({0.5f, 1});

    this->addChild(m_underline);

    return true;
}

void UnderlinedLabel::setOpacity(GLubyte opacity) {
    m_underline->setOpacity(opacity);
    m_label->setOpacity(opacity - 10);
}