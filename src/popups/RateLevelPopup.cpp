#include "RateLevelPopup.hpp"

#include "nodes/RatingSlider.hpp"
#include "nodes/AreaTextInput.hpp"
#include "nodes/UnderlinedLabel.hpp"

#include "actions/Shake.hpp"

RateLevelPopup::RateLevelPopup(int levelID) 
    : m_levelID(levelID) {}

RateLevelPopup* RateLevelPopup::create(GJGameLevel* level) {
    auto ret = new RateLevelPopup(level->m_levelID.value());

    if (ret->init(level->m_levelName)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool RateLevelPopup::init(std::string_view levelName) {
    if (!ALLManager::get().isLoggedIn() || m_levelID <= 0) {
        return false;
    }

    Popup::init(296, 237);

    this->setTitle(fmt::format("Rate {}", levelName), "goldFont.fnt", 0.7f, 21.f);
    
    auto container = CCNode::create();
    container->setAnchorPoint({0.5f, 0.5f});
    container->setPosition({m_size.width / 2.f, 120});
    container->setLayout(
        SimpleAxisLayout::create(Axis::Row)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setGap(12.f)
    );

    m_mainLayer->addChild(container);

    auto bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(60);
    bg->setContentSize({127, 155});

    container->addChild(bg);

    auto lbl = CCLabelBMFont::create("Rating", "bigFont.fnt");
    lbl->setOpacity(186);
    lbl->setScale(0.35f);
    lbl->setPosition({bg->getContentWidth() / 2.f, 145.5f});

    bg->addChild(lbl);

    auto line = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    line->setScaleX(0.25f);
    line->setScaleY(0.7f);
    line->setOpacity(90);
    line->setPosition({bg->getContentWidth() / 2.f, 135});

    bg->addChild(line);

    lbl = CCLabelBMFont::create("Your rating", "bigFont.fnt");
    lbl->setOpacity(121);
    lbl->setScale(0.325f);
    lbl->setPosition({bg->getContentWidth() / 2.f, 106});

    bg->addChild(lbl);
    
    m_ratingLabel = CCLabelBMFont::create("Not rated", "bigFont.fnt");
    m_ratingLabel->setColor({176, 221, 255});
    m_ratingLabel->setScale(0.45f);
    m_ratingLabel->setOpacity(197);
    m_ratingLabel->setPosition({bg->getContentWidth() / 2.f, 89.5f});

    bg->addChild(m_ratingLabel);

    auto underlinedLbl = UnderlinedLabel::create("Clear", "bigFont.fnt");
    underlinedLbl->setOpacity(80);

    m_clearRatingBtn = Button::createWithNode(underlinedLbl, [this](Button*) {
        m_currentRating.rated = false;
        m_currentRating.rating = 0.f;
        m_ratingLabel->setScale(0.45f);
        m_ratingLabel->setColor({176, 221, 255});
        m_ratingLabel->setOpacity(197);
        m_ratingLabel->setString("Not rated");
        m_ratingSlider->setValue(0.f);
    
        this->updateButtons();
    });
    m_clearRatingBtn->setScale(0.275f);
    m_clearRatingBtn->setScaleMultiplier(1.07f);
    m_clearRatingBtn->setPosition({bg->getContentWidth() / 2.f, 67});
    m_clearRatingBtn->setVisible(false);

    bg->addChild(m_clearRatingBtn);

    m_ratingSlider = RatingSlider::create([this](float value) {
        if (!m_currentRating.rated) {
            m_currentRating.rated = true;
            m_ratingLabel->setScale(0.65f);
            m_ratingLabel->setColor({155, 226, 248});
            m_ratingLabel->setOpacity(255);
            m_ratingLabel->setString("0.00");
            m_clearRatingBtn->setVisible(true);
        }

        m_currentRating.rating = value * 10.f;

        this->updateButtons();
    });
    m_ratingSlider->setPosition({bg->getContentWidth() / 2.f, 45.5f});

    bg->addChild(m_ratingSlider);

    auto smallLine = CCLayerColor::create({255, 255, 255, 60}, 1, 8);
    smallLine->ignoreAnchorPointForPosition(false);
    smallLine->setPosition({12, 32});

    bg->addChild(smallLine);

    smallLine = CCLayerColor::create({255, 255, 255, 60}, 1, 8);
    smallLine->ignoreAnchorPointForPosition(false);
    smallLine->setPosition({114, 32});

    bg->addChild(smallLine);

    auto linesContainer = CCNode::create();
    linesContainer->setPosition({bg->getContentWidth() / 2.f, 32});
    linesContainer->setAnchorPoint({0.5f, 0.5f});
    linesContainer->setLayout(
        SimpleAxisLayout::create(Axis::Row)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setGap(12.f)
    );

    bg->addChild(linesContainer);

    linesContainer->addChild(CCLayerColor::create({255, 255, 255, 30}, 1, 5));
    linesContainer->addChild(CCLayerColor::create({255, 255, 255, 30}, 1, 5));
    linesContainer->addChild(CCLayerColor::create({255, 255, 255, 30}, 1, 5));
    linesContainer->addChild(CCLayerColor::create({255, 255, 255, 30}, 1, 7));
    linesContainer->addChild(CCLayerColor::create({255, 255, 255, 30}, 1, 5));
    linesContainer->addChild(CCLayerColor::create({255, 255, 255, 30}, 1, 5));
    linesContainer->addChild(CCLayerColor::create({255, 255, 255, 30}, 1, 5));

    linesContainer->updateLayout();

    lbl = CCLabelBMFont::create("0", "bigFont.fnt");
    lbl->setOpacity(107);
    lbl->setScale(0.225f);
    lbl->setPosition({12.15f, 24});

    bg->addChild(lbl);

    lbl = CCLabelBMFont::create("10", "bigFont.fnt");
    lbl->setOpacity(107);
    lbl->setScale(0.225f);
    lbl->setPosition({114.15f, 24});
    
    bg->addChild(lbl);

    bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(60);
    bg->setContentSize({127, 155});

    container->addChild(bg);

    lbl = CCLabelBMFont::create("Review", "bigFont.fnt");
    lbl->setOpacity(186);
    lbl->setScale(0.35f);
    lbl->setPosition({bg->getContentWidth() / 2.f, 145.5f});

    bg->addChild(lbl);

    line = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    line->setScaleX(0.25f);
    line->setScaleY(0.7f);
    line->setOpacity(90);
    line->setPosition({bg->getContentWidth() / 2.f, 135});

    bg->addChild(line);

    m_areaInput = AreaTextInput::create({110, 114}, [this](const std::string& str) {
        if (str.length() > REVIEW_CHARACTER_LIMIT) {
            m_lengthLabel->setColor({255, 60, 60});
            m_lengthLabel->stopAllActions();
            m_lengthLabel->setPosition({122, 7});
            m_lengthLabel->runAction(CCSequence::create(
                Shake::create(0.25f, 0.52f),
                CallFuncExt::create([lbl = m_lengthLabel] {
                    lbl->setPosition({122, 7});
                }),
                nullptr
            ));
        } else {
            m_lengthLabel->setColor({255, 255, 255});
        }
        
        m_currentRating.review = str;

        m_lengthLabel->setString(fmt::format("{} / {}", std::clamp(static_cast<int>(m_currentRating.review.length()), 0, REVIEW_CHARACTER_LIMIT), REVIEW_CHARACTER_LIMIT).c_str());

        this->updateButtons();
    });
    m_areaInput->setPosition({bg->getContentWidth() / 2.f, 72});
        
    bg->addChild(m_areaInput);

    auto gradient = CCLayerGradient::create({117, 65, 39, 255}, {117, 65, 39, 0}, {0, 1});
    gradient->ignoreAnchorPointForPosition(false);
    gradient->setPosition({bg->getContentWidth() / 2.f, m_areaInput->getPositionY() - m_areaInput->getContentHeight() / 2.f + 6});
    gradient->setContentSize({bg->getContentWidth(), 12});

    bg->addChild(gradient);

    auto layer = CCLayerColor::create({117, 65, 39, 255}, bg->getContentWidth(), 5);
    layer->setPositionY(m_areaInput->getPositionY() - m_areaInput->getContentHeight() / 2.f - 5);

    bg->addChild(layer);

    layer = CCLayerColor::create({117, 65, 39, 255}, bg->getContentWidth(), 4);
    layer->setPositionY(m_areaInput->getPositionY() + m_areaInput->getContentHeight() / 2.f);

    bg->addChild(layer);

    m_inputGradient = CCLayerGradient::create({117, 65, 39, 255}, {117, 65, 39, 0}, {0, 1});
    m_inputGradient->ignoreAnchorPointForPosition(false);
    m_inputGradient->setPosition({bg->getContentWidth() / 2.f, m_areaInput->getPositionY() + m_areaInput->getContentHeight() / 2.f - 6});
    m_inputGradient->setContentSize({bg->getContentWidth(), 12});
    m_inputGradient->setScaleY(-1);

    bg->addChild(m_inputGradient);

    underlinedLbl = UnderlinedLabel::create("Clear", "bigFont.fnt");
    underlinedLbl->setOpacity(80);
    
    m_clearReviewBtn = Button::createWithNode(underlinedLbl, [this](Button*) {
        m_areaInput->setText("");
        m_areaInput->scrollToTop();
        m_lengthLabel->setString(fmt::format("0 / {}", REVIEW_CHARACTER_LIMIT).c_str());
        m_currentRating.review = "";
        this->updateButtons();
    });
    m_clearReviewBtn->setScale(0.26f);
    m_clearReviewBtn->setScaleMultiplier(1.07f);
    m_clearReviewBtn->setPosition({21, 9.25f});

    bg->addChild(m_clearReviewBtn);

    m_lengthLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_lengthLabel->setOpacity(95);
    m_lengthLabel->setAnchorPoint({1, 0.5f});
    m_lengthLabel->setScale(0.2f);
    m_lengthLabel->setPosition({122, 7});

    bg->addChild(m_lengthLabel);

    container->updateLayout();

    auto btnSpr = ButtonSprite::create("Submit");
    btnSpr->setCascadeOpacityEnabled(true);

    m_submitBtn = Button::createWithNode(btnSpr, [this](Button*) {
        m_fetching = false;

        this->setLoading(true);

        ALLManager::get().submitLevelRating(m_levelID, m_currentRating, [this, selfref = WeakRef(this)](Result<> res) {
            if (!selfref.lock()) {
                return;
            }

            this->setLoading(false);

            if (res.isOk()) {
                m_originalRating = m_currentRating;
                this->updateButtons();
            } else if (res.err().has_value()) {
                Notification::create(res.err().value(), NotificationIcon::Error)->show();
            } else {
                Notification::create("Failed to submit level rating", NotificationIcon::Error)->show();
            }
        });
    });
    m_submitBtn->setScaleMultiplier(1.13f);
    m_submitBtn->setScale(0.675f);
    m_submitBtn->setPosition({m_size.width / 2.f, 23});
    m_submitBtn->setOpacity(120);
    m_submitBtn->setEnabled(false);

    m_mainLayer->addChild(m_submitBtn);

    this->schedule(schedule_selector(RateLevelPopup::update), 1.f / 30.f, kCCRepeatForever, 0.15f);

    auto& all = ALLManager::get();
    
    if (all.isSubmittingRatingFor(m_levelID)) {
        this->setLoading(true);

        all.listenForSubmitLevelRating(m_levelID, [selfref = WeakRef(this)](Result<> res) {
            auto self = selfref.lock();

            if (!self) {
                return;
            }

            self->setLoading(false);

            self->m_originalRating = ALLManager::get().levelRatingForLevel(self->m_levelID).value();
            self->loadRating();
        });

        return true;
    }

    auto res = all.levelRatingForLevel(m_levelID);

    if (!res.has_value()) {
        m_fetching = true;
        
        this->setLoading(true);

        all.requestLevelRating(m_levelID, [selfref = WeakRef(this)](Result<const LevelRating&> res) {
            auto self = selfref.lock();

            if (!self) {
                return;
            }

            self->setLoading(false);

            if (!res.isOk()) {
                auto err = res.err().value_or("");

                if (!err.empty()) {
                    Notification::create(fmt::format("Failed to fetch current review and rating: {}", err), NotificationIcon::Error)->show();
                    self->onClose(nullptr);
                } else {
                    Notification::create("Failed to fetch current review and rating", NotificationIcon::Error)->show();
                }
            } else {
                self->m_originalRating = res.unwrap();
            }

            self->loadRating();
        });
    } else {
        m_originalRating = res.value();
        this->loadRating();
    }

    return true;
}

void RateLevelPopup::loadRating() {
    m_currentRating = m_originalRating;

    if (m_originalRating.rated) {
        m_ratingSlider->setValue(std::clamp(m_originalRating.rating / 10.f, 0.f, 1.f));
        m_ratingLabel->setScale(0.65f);
        m_ratingLabel->setColor({155, 226, 248});
        m_ratingLabel->setOpacity(255);
        m_ratingLabel->setString("0.00");
    } else {
        m_ratingLabel->setScale(0.45f);
        m_ratingLabel->setColor({176, 221, 255});
        m_ratingLabel->setOpacity(197);
        m_ratingLabel->setString("Not rated");
    }

    m_areaInput->setText(m_originalRating.review);
    m_areaInput->scrollToTop();

    m_lengthLabel->setString(fmt::format("{} / {}", m_originalRating.review.length(), REVIEW_CHARACTER_LIMIT).c_str());
    
    this->updateButtons();
}

void RateLevelPopup::updateButtons() {
    m_clearRatingBtn->setVisible(m_currentRating.rated);
    m_clearReviewBtn->setVisible(!m_currentRating.review.empty());
    m_submitBtn->setEnabled(m_originalRating != m_currentRating);
    m_submitBtn->setOpacity(m_originalRating != m_currentRating ? 255 : 120);
}

void RateLevelPopup::update(float) {
    m_inputGradient->setOpacity(static_cast<int>(std::clamp(m_areaInput->getScrollPosition() / 1.536974f, 0.f, 1.f) * 255.f));

    if (!m_currentRating.rated) {
        return;
    }

    auto targetValue = m_ratingSlider->getValue() * 10.f;

    if (m_currentValue == targetValue) {
        return;
    }

    if (targetValue > m_currentValue) {
        m_currentValue += 0.67f;
        
        if (m_currentValue > targetValue) {
            m_currentValue = targetValue;
        }

    } else {
        m_currentValue -= 0.67f;
        
        if (m_currentValue < targetValue) {
            m_currentValue = targetValue;
        }
    }

    auto color1 = ccColor3B{155, 226, 248};
    auto color2 = ccColor3B{33, 186, 233};
    auto t = m_currentValue / 10.f;
    auto r = color1.r + (color2.r - color1.r) * t;
    auto g = color1.g + (color2.g - color1.g) * t;
    auto b = color1.b + (color2.b - color1.b) * t;

    m_ratingLabel->setColor(ccc3(r, g, b));
    m_ratingLabel->setString(numToString(m_currentValue, 2).c_str());
}