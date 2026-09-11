#include "AreaTextInput.hpp"

#include "layers/ProScrollLayer.hpp"

AreaTextInput::AreaTextInput(Function<void(const std::string&)> callback)
    : m_callback(std::move(callback)) {}

AreaTextInput* AreaTextInput::create(const CCSize& size, Function<void(const std::string&)> callback) {
    auto ret = new AreaTextInput(std::move(callback));

    if (ret->init(size)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool AreaTextInput::init(const CCSize& size) {
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize(size);

    m_scroll = ProScrollLayer::create(size);
    m_scroll->setPosition(size / 2.f);
    m_scroll->m_contentLayer->setLayout(
        SimpleAxisLayout::create(Axis::Column)
            ->ignoreInvisibleChildren(false)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setMainAxisAlignment(MainAxisAlignment::Start)
    );
    m_scroll->setCallback([this](const CCPoint& pos) {
        if (m_enabled && isHoveringNode(pos, this)) {
            m_input->getInputNode()->m_textField->m_uCursorPos = this->cursorPosForClick(pos);
            m_input->focus();
        }
    });

    m_scroll->m_contentLayer->addChild(AxisGap::create(11.9f), 99);

    this->addChild(m_scroll);

    m_input = TextInput::create(size.width + 10.f, "");
    m_input->setPosition(size / 2.f);
    m_input->getBGSprite()->setVisible(false);
    m_input->getInputNode()->setVisible(false);
    m_input->getInputNode()->setContentHeight(size.height);
    m_input->setCommonFilter(CommonFilter::Any);
    m_input->setCallback([this](const std::string& str) {
        m_callback(str);

        m_cursor->setVisible(true);

        if (str.length() <= REVIEW_CHARACTER_LIMIT) {
            this->setText(str);
        } else {
            m_input->setString(m_text);
            m_input->getInputNode()->m_selected = true;
        }
    });

    this->addChild(m_input);

    m_placeholder = CCLabelBMFont::create("Write something...", "bigFont.fnt");
    m_placeholder->setAnchorPoint({0, 1});
    m_placeholder->setPosition({0.f, this->getContentHeight()});
    m_placeholder->setOpacity(89);
    m_placeholder->setScale(0.275f * m_scale);

    this->addChild(m_placeholder);

    auto clip = CCClippingNode::create();
    clip->setStencil(CCLayerColor::create({255, 255, 255, 255}, size.width, size.height));

    this->addChild(clip);

    m_cursor = CCLabelBMFont::create("l", "chatFont.fnt");
    m_cursor->setOpacity(120);
    m_cursor->setScaleX(0.54f * m_scale);
    m_cursor->setScaleY(0.63f * m_scale);
    m_cursor->runAction(CCRepeatForever::create(
        CCSequence::create(
            CCDelayTime::create(0.5f),
            CCHide::create(),
            CCDelayTime::create(0.5f),
            CCShow::create(),
            nullptr
        )
    ));

    clip->addChild(m_cursor);
    
    this->schedule(schedule_selector(AreaTextInput::update), 1.f / 30.f, kCCRepeatForever, 0.f);

    return true;
}

void AreaTextInput::setText(const std::string& text) {
    m_input->setString(text);

    m_text = text;

    if (text.empty()) {
        if (m_container) {
            m_container->removeFromParent();
            m_container = nullptr;
        }

        return;
    }

    if (!m_container) {
        m_container = CCNode::create();
        m_container->setAnchorPoint({0.5f, 1});
        m_container->setContentWidth(this->getContentWidth());
        m_container->setLayout(
            SimpleAxisLayout::create(Axis::Column)
                ->setMainAxisScaling(AxisScaling::Grow)
                ->setCrossAxisScaling(AxisScaling::None)
                ->setMainAxisAlignment(MainAxisAlignment::Start)
                ->setCrossAxisAlignment(CrossAxisAlignment::Start)
        );

        m_scroll->m_contentLayer->addChild(m_container);
    }

    if (text.empty()) {
        m_container->removeAllChildren();
        m_container->updateLayout();

        m_scroll->m_contentLayer->updateLayout();
        
        return;
    }
    
    auto labels = m_container->getChildrenExt<CCLabelBMFont*>();
    auto lineIndex = 0;
    auto remainingText = text;

    while (!remainingText.empty()) {
        CCLabelBMFont* lbl = nullptr;
        
        if (lineIndex < labels.size()) {
            lbl = labels[lineIndex];
        } else {
            lbl = CCLabelBMFont::create("", "bigFont.fnt");
            lbl->setOpacity(m_opacity);
            lbl->setScale(m_scale * 0.25f);
            
            m_container->addChild(lbl);
        }

        lbl->setString(remainingText.c_str());

        auto didWrap = false;
        auto i = 0;
        
        for (auto letter : lbl->getChildrenExt()) {
            if (i >= remainingText.size()) {
                break;
            }

            auto x = letter->getPositionX() * lbl->getScale();
            auto offset = remainingText[i] == ' ' ? (15.f * lbl->getScale()) : ((letter->getContentWidth() * lbl->getScale()) / 2.f);

            if (x + offset > m_container->getContentWidth()) {
                auto pos = remainingText.rfind(' ', i);
                auto cutWords = false;

                if (pos == std::string::npos) {
                    pos = i <= 0 ? 1 : i;
                    cutWords = true;
                }

                lbl->setString(remainingText.substr(0, pos + (cutWords ? 0 : 1)).c_str());
                
                remainingText = remainingText.substr(pos + (cutWords ? 0 : 1));
                didWrap = true;

                break;
            }

            i++;
        }

        if (!didWrap) {
            remainingText.clear();
        }

        if (lbl->getChildren()) {
            auto children = lbl->getChildrenExt();

            for (int j = children.size() - 1; j >= 0; j--) {
                if (!children[j]->isVisible()) {
                    children[j]->removeFromParent();
                }
            }
        }

        lineIndex++;
    }

    auto leftOvers = m_container->getChildrenExt<CCLabelBMFont*>();
    for (int i = lineIndex; i < leftOvers.size(); i++) {
        leftOvers[i]->removeFromParent();
    }

    m_container->updateLayout();
    m_scroll->m_contentLayer->updateLayout();
}

CCPoint AreaTextInput::getCursorPos(int pos) {
    if (!m_container || m_container->getChildrenCount() <= 0) {
        return CCPoint{0, this->getContentHeight() - 5.f};
    }

    if (pos == -1) {
        auto children = m_container->getChildrenExt();
        auto label = children[std::min(static_cast<unsigned int>(children.size() - 1), m_container->getChildrenCount() - 1)];
        auto letter = label->getChildrenExt()[std::min(children.back()->getChildrenCount() - 1, label->getChildrenCount() - 1)];

        return this->convertToNodeSpace({
            letter->convertToWorldSpace(
                {(letter->getContentWidth() > 0.f ? letter->getContentWidth() : 15.f) - 3.75f, 0.f}
            ).x,
            letter->getParent()->convertToWorldSpace({0.f, letter->getParent()->getContentHeight() / 2.f - 2.3f}).y
        });
    }
    
    auto count = 0;

    for (auto label : m_container->getChildrenExt()) {
        if (label->getChildrenCount() + count < pos) {
            count += label->getChildrenCount();
            continue;
        }

        if (pos - count <= 0) {
            return CCPoint{0, this->getContentHeight() - 5.f};
        }

        auto letter = label->getChildrenExt()[std::min(static_cast<unsigned int>(pos - count - 1), label->getChildrenCount() - 1)];

        return this->convertToNodeSpace({
            letter->convertToWorldSpace(
                {(letter->getContentWidth() > 0.f ? letter->getContentWidth() : 15.f) - 3.75f, 0.f}
            ).x,
            letter->getParent()->convertToWorldSpace({0.f, letter->getParent()->getContentHeight() / 2.f - 2.3f}).y
        });
    }

    return {0, 0};
}

int AreaTextInput::cursorPosForClick(const CCPoint& pos) {
    if (!m_container) {
        return -1;
    }

    auto children = m_container->getChildrenExt<CCLabelBMFont*>();
    
    if (children.empty()) {
        return -1;
    }

    CCNode* closestLabel = nullptr;
    auto minDistance = 0.f;
    auto count = 0;

    for (auto label : children) {        
        auto distance = ccpDistance(pos, {pos.x, label->convertToWorldSpace({0, label->getContentHeight() / 2.f}).y});
        
        if (!closestLabel || distance < minDistance) {
            minDistance = distance;
            closestLabel = label;
        }
    }

    if (m_container->convertToNodeSpace(pos).y < m_container->convertToNodeSpace(closestLabel->convertToWorldSpace({0, 0})).y) {
        return -1;
    }

    for (auto label : children) {
        if (label == closestLabel) {
            break;
        }
     
        count += label->getChildrenCount();
    }

    if (!closestLabel) {
        return -1;
    }

    CCNode* closestLetter = nullptr;

    for (auto letter : closestLabel->getChildrenExt()) {
        auto distance = ccpDistance(pos, letter->convertToWorldSpace(letter->getScaledContentSize() / 2.f));
        
        if (!closestLetter || distance < minDistance) {
            minDistance = distance;
            closestLetter = letter;
        }
    }

    for (auto letter : closestLabel->getChildrenExt()) {
        if (letter == closestLetter) {
            break;
        }

        count++;
    }

    if (!closestLetter) {
        return -1;
    }

    if (!(closestLetter->convertToNodeSpace(pos).x < (closestLetter->getContentWidth() > 0.f ? closestLetter->getContentWidth() : 15.f) / 2.f)) {
        count++;
    }

    return count;
}

void AreaTextInput::update(float) {
    auto pos = m_input->getInputNode()->m_textField->m_uCursorPos;
    m_cursor->setPosition(this->getCursorPos(pos));

    if (pos != m_cursorPos) {
        m_cursor->stopAllActions();
        m_cursor->setVisible(true);
        m_cursor->runAction(CCRepeatForever::create(
            CCSequence::create(
                CCDelayTime::create(0.5f),
                CCHide::create(),
                CCDelayTime::create(0.5f),
                CCShow::create(),
                nullptr
            )
        ));
    }

    m_cursorPos = pos;

    m_placeholder->setVisible(!m_input->getInputNode()->m_selected && m_text.empty());
    m_cursor->setOpacity(m_input->getInputNode()->m_selected ? 120 : 0);
}

float AreaTextInput::getScrollPosition() {
    return m_scroll->m_contentLayer->getPositionY() + m_scroll->m_contentLayer->getContentHeight() - this->getContentHeight();
}

void AreaTextInput::scrollToTop() {
    m_scroll->scrollToTop();
}

void AreaTextInput::setEnabled(bool enabled) {
    m_enabled = enabled;
    m_input->setEnabled(m_enabled);
}

void AreaTextInput::setOpacity(int opacity) {
    m_opacity = opacity;

    if (!m_text.empty()) {
        this->setText(m_text);
    }
}

void AreaTextInput::setScale(float scale) {
    m_scale = scale;

    if (!m_text.empty()) {
        this->setText(m_text);
    }
}

float AreaTextInput::getScale() {
    return 1.f;
}