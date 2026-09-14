#include "RankLevelPopup.hpp"
#include "ALLManager.hpp"

#include "layers/PickDifficultyLayer.hpp"
#include "layers/LevelVersusLayer.hpp"
#include "layers/RankResultLayer.hpp"

RankLevelPopup::RankLevelPopup(int levelID, std::string levelName)
    : m_levelID(levelID), m_levelName(std::move(levelName)) {}

RankLevelPopup* RankLevelPopup::create(GJGameLevel* level) {
    auto ret = new RankLevelPopup(level->m_levelID.value(), std::string(level->m_levelName));

    if (ret->init(level)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool RankLevelPopup::init(GJGameLevel* level) {
    if (!ALLManager::get().isLoggedIn() || m_levelID <= 0) {
        return false;
    }

    auto& all = ALLManager::get();

    Popup::init(300, 280);

    this->setTitle(fmt::format("Rank {}", m_levelName), "goldFont.fnt", 0.7f, 21.f);

    m_instructionLabel = CCLabelBMFont::create("Pick a difficulty to compare within.", "bigFont.fnt");
    m_instructionLabel->setPosition({m_size.width / 2.f, 236});
    m_instructionLabel->setOpacity(177);
    m_instructionLabel->limitLabelWidth(m_size.width - 23.f, 0.325f, 0.f);

    m_mainLayer->addChild(m_instructionLabel);

    auto bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(26);
    bg->setPosition({m_size.width / 2.f, 132});
    bg->setContentSize({260, 185});

    m_mainLayer->addChild(bg);

    m_overlay = NineSlice::create("square02b_001.png");
    m_overlay->setColor({0, 0, 0});
    m_overlay->setOpacity(0);
    m_overlay->setPosition({m_size.width / 2.f, 132});
    m_overlay->setContentSize({260, 185});

    m_mainLayer->addChild(m_overlay, 10);

    auto stencil = NineSlice::create("square02b_001.png");
    stencil->setPosition({m_size.width / 2.f, 132});
    stencil->setContentSize({260, 185});

    auto clip = CCClippingNode::create();
    clip->setAlphaThreshold(0.01f);
    clip->setStencil(stencil);
    
    m_mainLayer->addChild(clip);

    if (!all.areUserListsCached()) {
        this->setLoading(true);
    }

    m_pickDifficultyLayer = PickDifficultyLayer::create(
        m_levelID,
        [this](Result<> res) {
            if (res.isOk()) {
                this->setLoading(false);
            } else {
                this->onClose(nullptr);
                Notification::create("Failed to fetch user lists", NotificationIcon::Error)->show();
            }
        },
        [this](int listID, Difficulty difficulty, std::vector<LevelRanking> levels) {
            if (m_currentStep != 0) {
                return;
            }

            m_currentStep = 1;

            this->updateStep();

            if (!levels.empty() || ALLManager::get().getAnchorLevels().has_value()) {
                m_levelVersusLayer->setLevels(listID, difficulty, std::move(levels));
            } else {
                m_overlay->stopAllActions();
                m_overlay->runAction(CCFadeTo::create(0.15f, 100));
                this->runAction(CCSequence::create(
                    CCDelayTime::create(0.2f),
                    CallFuncExt::create([this, listID, difficulty, levels = std::move(levels)] {
                        m_levelVersusLayer->setLevels(listID, difficulty, std::move(levels));
                    }),
                    nullptr
                ));
            }
        }
    );
    m_pickDifficultyLayer->setPosition(bg->getPosition());

    clip->addChild(m_pickDifficultyLayer);

    m_levelVersusLayer = LevelVersusLayer::create(
        level,
        [this](Result<bool> state) {
            if (state.isOk()) {
                this->setLoading(state.unwrap());

                if (!state.unwrap()) {
                    m_overlay->stopAllActions();
                    m_overlay->runAction(CCFadeTo::create(0.15f, 0));
                }
            } else {
                m_overlay->stopAllActions();
                m_overlay->runAction(CCFadeTo::create(0.15f, 0));
                this->setLoading(false);
                m_currentStep = 0;
                this->updateStep();
                Notification::create("Failed to fetch anchor levels", NotificationIcon::Error)->show();
            }
        },
        [this](int64_t id, bool harder, int64_t refAbove, int64_t refBelow, int listID, Difficulty difficulty) {
            m_placement = 0;

            if (m_currentStep != 1) {
                return;
            }

            m_currentStep = 2;
            
            if (ALLManager::get().areUserListsCached()) {
                m_rankResultLayer->setLevel(id, harder, refAbove, refBelow, listID, difficulty);
                m_placement = m_rankResultLayer->getPlacement();
            } else {
                m_overlay->stopAllActions();
                m_overlay->runAction(CCFadeTo::create(0.15f, 100));
                this->runAction(CCSequence::create(
                    CCDelayTime::create(0.2f),
                    CallFuncExt::create([this, id, harder, difficulty, refAbove, listID, refBelow] {
                        m_rankResultLayer->setLevel(id, harder, refAbove, refBelow, listID, difficulty);

                        m_instructionLabel->stopAllActions();
                        m_instructionLabel->runAction(CCSequence::create(
                            CCFadeTo::create(0.07f, 90),
                            CallFuncExt::create([this] {
                                if (m_rankResultLayer->getPlacement() != m_placement) {
                                    m_instructionLabel->setString(fmt::format("{} will be placed at #{}", m_levelName, m_rankResultLayer->getPlacement()).c_str());
                                    m_instructionLabel->limitLabelWidth(m_size.width - 23.f, 0.325f, 0.f);
                                }
                            }),
                            CCFadeTo::create(0.07f, 140),
                            nullptr
                        ));
                    }),
                    nullptr
                ));
            }

            this->updateStep();
        }
    );
    m_levelVersusLayer->setPosition(bg->getPosition() + CCPoint{m_levelVersusLayer->getContentWidth(), 0});

    clip->addChild(m_levelVersusLayer);

    m_rankResultLayer = RankResultLayer::create(
        level,
        [this](Result<bool> state) {
            if (state.isOk()) {
                this->setLoading(state.unwrap());

                if (!state.unwrap()) {
                    m_overlay->stopAllActions();
                    m_overlay->runAction(CCFadeTo::create(0.15f, 0));
                }
            } else {
                m_overlay->stopAllActions();
                m_overlay->runAction(CCFadeTo::create(0.15f, 0));
                this->setLoading(false);
                m_currentStep = 1;
                this->updateStep();
                Notification::create("Failed to fetch user lists", NotificationIcon::Error)->show();
            }
        }
    );
    m_rankResultLayer->setPosition(bg->getPosition() + CCPoint{m_rankResultLayer->getContentWidth() * 2, 0});

    clip->addChild(m_rankResultLayer);

    auto btnSpr = ButtonSprite::create("Back", 77, 0, 1.f, true);
    btnSpr->setCascadeOpacityEnabled(true);

    m_backButton = Button::createWithNode(btnSpr, [this](Button*) {
        this->onBack();
    });
    m_backButton->setScaleMultiplier(1.13f);
    m_backButton->setPosition({51, 22});
    m_backButton->setScale(0.68f);
    m_backButton->setOpacity(120);
    m_backButton->setEnabled(false);

    m_mainLayer->addChild(m_backButton);

    btnSpr = ButtonSprite::create("Place", 77, 0, 1.f, true);
    btnSpr->setCascadeOpacityEnabled(true);

    m_placeButton = Button::createWithNode(btnSpr, [this](Button*) {
        if (m_isLoading) {
            return;
        }

        m_submitting = true;
        this->setLoading(true);

        m_rankResultLayer->submitPlacement([this, selfref = WeakRef(this)](Result<> res) {
            if (!selfref.lock()) {
                return;
            }

            m_submitting = false;
            this->setLoading(false);

            if (res.isOk()) {
                this->onClose(nullptr);
                Notification::create("Placement submitted", NotificationIcon::Success)->show();
                return;
            }

            Notification::create(res.err().value_or("Failed to submit placement"), NotificationIcon::Error)->show();
        });
    });
    m_placeButton->setScaleMultiplier(1.13f);
    m_placeButton->setPosition({249, 22});
    m_placeButton->setScale(0.68f);
    m_placeButton->setOpacity(120);
    m_placeButton->setEnabled(false);

    m_mainLayer->addChild(m_placeButton);

    return true;
}

void RankLevelPopup::updateStep() {
    m_backButton->setOpacity(m_currentStep > 0 ? 255 : 120);
    m_backButton->setEnabled(m_currentStep > 0);
    
    m_placeButton->setOpacity(m_currentStep >= 2 ? 255 : 120);
    m_placeButton->setEnabled(m_currentStep >= 2);

    m_pickDifficultyLayer->stopAllActions();
    m_levelVersusLayer->stopAllActions();

    auto text = std::string{};

    switch (m_currentStep) {
        default:
        case 0: {
            m_pickDifficultyLayer->runAction(CCEaseSineOut::create(CCMoveTo::create(0.25f, {
                m_size.width / 2.f, m_pickDifficultyLayer->getPositionY()
            })));

            m_levelVersusLayer->runAction(CCEaseSineIn::create(CCMoveTo::create(0.15f, {
                m_size.width / 2.f + m_levelVersusLayer->getContentWidth(), m_levelVersusLayer->getPositionY()
            })));

            m_rankResultLayer->runAction(CCEaseSineIn::create(CCMoveTo::create(0.15f, {
                m_size.width / 2.f + m_rankResultLayer->getContentWidth() * 2, m_rankResultLayer->getPositionY()
            })));

            text = "Pick a difficulty to compare within.";

            break;
        }
        case 1: {
            m_pickDifficultyLayer->runAction(CCEaseSineIn::create(CCMoveTo::create(0.2f, {
                -m_pickDifficultyLayer->getContentWidth(), m_pickDifficultyLayer->getPositionY()
            })));

            m_levelVersusLayer->runAction(CCEaseSineOut::create(CCMoveTo::create(0.25f, {
                m_size.width / 2.f, m_levelVersusLayer->getPositionY()
            })));

            m_rankResultLayer->runAction(CCEaseSineIn::create(CCMoveTo::create(0.15f, {
                m_size.width / 2.f + m_rankResultLayer->getContentWidth(), m_rankResultLayer->getPositionY()
            })));

            text = "Which level is harder?";

            break;
        }
        case 2: {
            m_pickDifficultyLayer->runAction(CCEaseSineIn::create(CCMoveTo::create(0.15f, {
                -m_pickDifficultyLayer->getContentWidth() * 2, m_pickDifficultyLayer->getPositionY()
            })));

            m_levelVersusLayer->runAction(CCEaseSineIn::create(CCMoveTo::create(0.25f, {
                -m_levelVersusLayer->getContentWidth(), m_levelVersusLayer->getPositionY()
            })));

            m_rankResultLayer->runAction(CCEaseSineOut::create(CCMoveTo::create(0.3f, {
                m_size.width / 2.f, m_rankResultLayer->getPositionY()
            })));

            text = fmt::format("{} will be placed at #{}", m_levelName, m_placement > 0 ? numToString(m_placement) : "-");

            break;
        }
    };

    m_instructionLabel->stopAllActions();
    m_instructionLabel->runAction(CCSequence::create(
        CCFadeTo::create(0.07f, 90),
        CallFuncExt::create([this, text] {
            m_instructionLabel->setString(text.c_str());
            m_instructionLabel->limitLabelWidth(m_size.width - 23.f, 0.325f, 0.f);
        }),
        CCFadeTo::create(0.07f, 140),
        nullptr
    ));
}

void RankLevelPopup::onBack() {
    if (m_currentStep == 1) {
        if (!m_levelVersusLayer->onBack()) {
            return;
        }
    }

    if (m_currentStep <= 0) {
        return;
    }

    m_currentStep--;

    this->updateStep();
}

void RankLevelPopup::keyDown(enumKeyCodes key, double timestamp) {
    if (key == enumKeyCodes::KEY_Escape && m_backButton->isEnabled()) {
        this->onBack();
        return;
    }

    return Popup::keyDown(key, timestamp);
}