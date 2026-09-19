#include "LevelVersusLayer.hpp"
#include "ALLManager.hpp"

#include "nodes/LevelVersusNode.hpp"
#include "actions/SizeTo.hpp"

#include <Geode/ui/Button.hpp>
#include <vector>

LevelVersusLayer::LevelVersusLayer(Function<void(Result<bool>)> loadCallback, Function<void(int64_t, bool, int64_t, int64_t, int, Difficulty)> finishCallback)
    : m_loadCallback(std::move(loadCallback)), m_finishCallback(std::move(finishCallback)) {}

LevelVersusLayer* LevelVersusLayer::create(GJGameLevel* level, Function<void(Result<bool>)> loadCallback, Function<void(int64_t, bool, int64_t, int64_t, int, Difficulty)> finishCallback) {
    auto ret = new LevelVersusLayer(std::move(loadCallback), std::move(finishCallback));

    if (ret->init(level)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool LevelVersusLayer::init(GJGameLevel* level) {
    m_levelID = level->m_levelID.value();

    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({260, 185});

    auto lbl = CCLabelBMFont::create("vs", "bigFont.fnt");
    lbl->setScale(0.47f);
    lbl->setOpacity(190);
    lbl->setPosition({this->getContentWidth() / 2.f + 0.5f, 121});

    this->addChild(lbl);

    m_mainNode = LevelVersusNode::create(LevelRanking{
        .id = level->m_levelID.value(),
        .name = level->m_levelName,
        .bucket = difficultyForLevel(level)
    });
    auto btn = Button::createWithNode(m_mainNode, [this](Button*) {
        if (!m_animating) {
            m_duelHistory.push_back(Duel{
                true,
                m_refAbove,
                m_levels[m_lastIndex].id
            });

            this->loadDuel(m_currentDuel + 1, TransitionMode::Down);
        }
    });
    btn->setPosition({61, 119});
    btn->setScaleMultiplier(1.05f);

    this->addChild(btn);

    auto stencil = CCLayerColor::create({255, 255, 255, 255}, 115, 122);
    stencil->setPosition({140, 58});

    auto clip = CCClippingNode::create();
    clip->setStencil(stencil);

    this->addChild(clip);

    m_nodesContainer = CCNode::create();
    m_nodesContainer->setAnchorPoint({0.5f, 0.5f});
    m_nodesContainer->setLayout(
        SimpleAxisLayout::create(Axis::Column)
            ->setGap(9.f)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setCrossAxisScaling(AxisScaling::Grow)
    );
    m_nodesContainer->setPosition({200, 160});

    clip->addChild(m_nodesContainer);

    auto node = LevelVersusNode::create();

    m_nodes[0] = node;

    m_nodesContainer->addChild(node);

    node = LevelVersusNode::create();

    btn = Button::createWithNode(node, [this](Button*) {
        if (!m_animating) {
            m_duelHistory.push_back(Duel{
                false,
                m_levels[m_lastIndex].id,
                m_refBelow
            });
            
            this->loadDuel(m_currentDuel + 1, TransitionMode::Up);
        }
    });
    btn->setScaleMultiplier(1.05f);

    m_nodes[1] = node;
    
    m_nodesContainer->addChild(btn);

    node = LevelVersusNode::create();

    m_nodes[2] = node;

    m_nodesContainer->addChild(node);

    node = LevelVersusNode::create();

    m_nodes[3] = node;

    m_nodesContainer->addChild(node);

    m_nodesContainer->updateLayout();

    auto gradient = CCLayerGradient::create({137, 76, 46, 255}, {137, 76, 46, 0});
    gradient->setContentSize({115, 12});
    gradient->setPosition({140, 58 + stencil->getContentHeight() - gradient->getContentHeight()});

    this->addChild(gradient);

    gradient = CCLayerGradient::create({137, 76, 46, 255}, {137, 76, 46, 0}, {0, 1});
    gradient->setContentSize({115, 12});
    gradient->setPosition({140, 58});

    this->addChild(gradient);

    auto bar = NineSlice::create("square02b_001.png");
    bar->setContentSize({420, 17});
    bar->setScale(0.6f);
    bar->setPosition({this->getContentWidth() / 2.f, 9});
    bar->setColor({0, 0, 0});
    bar->setOpacity(35);

    this->addChild(bar);

    m_duelLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_duelLabel->setScale(0.25f);
    m_duelLabel->setAnchorPoint({0, 0.5f});
    m_duelLabel->setPosition({5, 20});
    m_duelLabel->setOpacity(114);

    this->addChild(m_duelLabel);

    lbl = CCLabelBMFont::create("Too close", "bigFont.fnt");
    lbl->setScale(0.23f);
    lbl->setPosition({130, 47});
    lbl->setOpacity(110);

    this->addChild(lbl);

    auto btnBg = NineSlice::create("GJ_button_01.png");
    btnBg->setContentSize({40, 30});

    auto spr = CCSprite::create("unsure.png"_spr);
    spr->setScale(0.6f);
    spr->setPosition(btnBg->getContentSize() / 2.f + CCPoint{0, 0.5f});

    btnBg->addChild(spr);

    btn = Button::createWithNode(btnBg, [this](Button*) {
        auto currentHalf = std::vector<int>{};
    
        for (int i = 0; i < m_levels.size(); i++) {
            currentHalf.push_back(i);
        }

        for (const auto& duel : m_duelHistory) {
            if (duel.choseHarder) {
                currentHalf = std::vector<int>(currentHalf.begin(), currentHalf.begin() + currentHalf.size() / 2);
            } else {
                currentHalf = std::vector<int>(currentHalf.begin() + currentHalf.size() / 2 + 1, currentHalf.end());
            }
        }

        auto index = currentHalf[currentHalf.size() / 2.f];
        m_finishCallback(m_levels[index].id, true, m_refAbove, m_refBelow, m_listID, m_difficulty);
    });
    btn->setScale(0.65f);
    btn->setScaleMultiplier(1.12f);
    btn->setPosition({130, 32});

    this->addChild(btn);

    m_barStencil = NineSlice::create("square02b_001.png");
    m_barStencil->setContentSize({0, 17});
    m_barStencil->setScale(0.6f);
    m_barStencil->setAnchorPoint({0, 0.5f});
    m_barStencil->setPosition({this->getContentWidth() / 2.f - bar->getScaledContentWidth() / 2.f, 9});

    clip = CCClippingNode::create();
    clip->setAlphaThreshold(0.01f);
    clip->setStencil(m_barStencil);

    this->addChild(clip);

    m_gradientBar = CCLayerGradient::create({240, 106, 76, 255}, {245, 185, 66, 255}, {1, 0});
    m_gradientBar->setAnchorPoint({0, 0.5f});
    m_gradientBar->ignoreAnchorPointForPosition(false);
    m_gradientBar->setScale(bar->getScale());
    m_gradientBar->setContentSize(m_barStencil->getContentSize());
    m_gradientBar->setPosition(m_barStencil->getPosition());

    clip->addChild(m_gradientBar);

    this->scheduleUpdate();

    return true;
}

void LevelVersusLayer::loadDuel(int duel, TransitionMode transition) {
    if ((duel == m_currentDuel && transition != TransitionMode::Instant) || m_levels.empty()) {
        return;
    }

    m_duelHistory.erase(m_duelHistory.begin() + duel, m_duelHistory.end());

    if (!m_duelHistory.empty()) {
        m_refAbove = m_duelHistory.back().refAbove;
        m_refBelow = m_duelHistory.back().refBelow;
    } else {
        m_refAbove = 0;
        m_refBelow = 0;
    }

    auto currentHalf = std::vector<int>{};
    
    for (int i = 0; i < m_levels.size(); i++) {
        currentHalf.push_back(i);
    }

    for (const auto& duel : m_duelHistory) {
        if (duel.choseHarder) {
            currentHalf = std::vector<int>(currentHalf.begin(), currentHalf.begin() + currentHalf.size() / 2);
        } else {
            currentHalf = std::vector<int>(currentHalf.begin() + currentHalf.size() / 2 + 1, currentHalf.end());
        }
    }

    if (currentHalf.empty()) {
        m_finishCallback(m_levels[m_lastIndex].id, m_duelHistory.back().choseHarder, m_refAbove, m_refBelow, m_listID, m_difficulty);
        
        m_duelHistory.pop_back();

        if (!m_duelHistory.empty()) {
            m_refAbove = m_duelHistory.back().refAbove;
            m_refBelow = m_duelHistory.back().refBelow;
        } else {
            m_refAbove = 0;
            m_refBelow = 0;
        }

        return;
    }

    if (m_currentDuel != duel) {
        m_currentDuel = duel;

        auto totalDuels = std::ceil(std::log2(m_levels.size() + 1));

        m_duelLabel->stopAllActions();
        m_duelLabel->runAction(CCSequence::create(
            CCFadeTo::create(0.07f, 90),
            CallFuncExt::create([this, totalDuels] {
                m_duelLabel->setString(
                    fmt::format("Duel {} of ~{}", m_currentDuel + 1, totalDuels).c_str()
                );
            }),
            CCFadeTo::create(0.07f, 140),
            nullptr
        ));
        
        m_gradientBar->stopAllActions();
        m_gradientBar->runAction(
            CCEaseSineOut::create(SizeTo::create(0.23f, {std::clamp(static_cast<float>((m_currentDuel + 1) / totalDuels), 0.f, 1.f) * 420.f, m_gradientBar->getContentHeight()}))
        );

        m_barStencil->stopAllActions();
        m_barStencil->runAction(
            CCEaseSineOut::create(SizeTo::create(0.23f, {std::clamp(static_cast<float>((m_currentDuel + 1) / totalDuels), 0.f, 1.f) * 420.f, m_gradientBar->getContentHeight()}))
        );
    }

    auto index = currentHalf[currentHalf.size() / 2];

    if (transition == TransitionMode::Instant) {
        m_lastIndex = index;

        m_nodesContainer->setPositionY(78);

        if (index == 0) {
            m_nodes[0]->setHidden(true, true);
        } else {
            m_nodes[0]->setLevel(m_levels[index - 1]);
            m_nodes[0]->setHidden(false, true);
            m_nodes[0]->setMode(LevelVersusNode::Mode::Bottom, true);
        }

        m_nodes[1]->setLevel(m_levels[index]);
        m_nodes[1]->setHidden(false, true);
        m_nodes[1]->setMode(LevelVersusNode::Mode::Main, true);

        if (index == static_cast<int>(m_levels.size()) - 1) {
            m_nodes[2]->setHidden(true, true);
        } else {
            m_nodes[2]->setLevel(m_levels[index + 1]);
            m_nodes[2]->setHidden(false, true);
            m_nodes[2]->setMode(LevelVersusNode::Mode::Top, true);
        }

        m_nodes[3]->setHidden(false, true);

        return;
    }

    bool up = transition == TransitionMode::Up;

    if (up) {
        m_animating = true;

        if (index == 0) {
            m_nodes[1]->setHidden(true, false);
        } else {
            m_nodes[1]->setLevel(m_levels[index - 1], true);
            m_nodes[1]->setHidden(false, false);
            m_nodes[1]->setMode(LevelVersusNode::Mode::Bottom, false);
        }

        m_nodes[2]->setLevel(m_levels[index], true, true);
        m_nodes[2]->setHidden(false, false);
        m_nodes[2]->setMode(LevelVersusNode::Mode::Main, false);

        if (index == static_cast<int>(m_levels.size()) - 1) {
            m_nodes[3]->setHidden(true, false);
        } else {
            m_nodes[3]->setLevel(m_levels[index + 1], true);
            m_nodes[3]->setHidden(false, false);
            m_nodes[3]->setMode(LevelVersusNode::Mode::Top, false);
        }

        m_nodesContainer->stopAllActions();
        m_nodesContainer->setPositionY(78);
        m_nodesContainer->runAction(CCSequence::create(
            CCEaseSineInOut::create(CCMoveBy::create(0.27f, {0, 82})),
            CallFuncExt::create([this] {
                this->loadDuel(m_currentDuel, TransitionMode::Instant);
                m_animating = false;
            }),
            nullptr
        ));

        m_lastIndex = index;

        return;
    }

    m_animating = true;

    if (m_lastIndex == 0) {
        m_nodes[1]->setHidden(true, true);
    } else {
        m_nodes[1]->setLevel(m_levels[m_lastIndex - 1]);
        m_nodes[1]->setHidden(false, true);
        m_nodes[1]->setMode(LevelVersusNode::Mode::Bottom, true);
    }

    m_nodes[2]->setLevel(m_levels[m_lastIndex]);
    m_nodes[2]->setHidden(false, true);
    m_nodes[2]->setMode(LevelVersusNode::Mode::Main, true);

    if (m_lastIndex == static_cast<int>(m_levels.size()) - 1) {
        m_nodes[3]->setHidden(true, true);
    } else {
        m_nodes[3]->setLevel(m_levels[m_lastIndex + 1]);
        m_nodes[3]->setHidden(false, true);
        m_nodes[3]->setMode(LevelVersusNode::Mode::Top, true);
    }

    if (index == 0) {
        m_nodes[0]->setHidden(true, false);
    } else {
        m_nodes[0]->setLevel(m_levels[index - 1], true);
        m_nodes[0]->setHidden(false, false);
        m_nodes[0]->setMode(LevelVersusNode::Mode::Bottom, false);
    }

    m_nodes[1]->setLevel(m_levels[index], true, true);
    m_nodes[1]->setHidden(false, false);
    m_nodes[1]->setMode(LevelVersusNode::Mode::Main, false);

    if (index == static_cast<int>(m_levels.size()) - 1) {
        m_nodes[2]->setHidden(true, false);
    } else {
        m_nodes[2]->setLevel(m_levels[index + 1], true);
        m_nodes[2]->setHidden(false, false);
        m_nodes[2]->setMode(LevelVersusNode::Mode::Top, false);
    }

    m_nodesContainer->stopAllActions();
    m_nodesContainer->setPositionY(160);
    m_nodesContainer->runAction(CCSequence::create(
        CCEaseSineInOut::create(CCMoveBy::create(0.27f, {0, -82})),
        CallFuncExt::create([this] {
            this->loadDuel(m_currentDuel, TransitionMode::Instant);
            m_animating = false;
        }),
        nullptr
    ));

    m_lastIndex = index;
}

void LevelVersusLayer::update(float) {
    auto mousePos = getMousePos();
    m_mainNode->setHovering(isHoveringNode(mousePos, m_mainNode));
    m_nodes[1]->setHovering(isHoveringNode(mousePos, m_nodes[1]));
}

void LevelVersusLayer::setLevels(int listID, Difficulty difficulty, std::vector<LevelRanking> levels) {
    m_levels = std::move(levels);
    m_difficulty = difficulty;
    m_listID = listID;

    std::erase_if(m_levels, [this](const LevelRanking& level) {
        return level.id == m_levelID;
    });

    if (!m_levels.empty()) {
        m_usingRefs = false;
        m_loadCallback(Ok(false));
        this->loadDuel(0, TransitionMode::Instant);
        return;
    }

    m_usingRefs = true;

    auto& all = ALLManager::get();
    auto res = all.getAnchorLevels();

    if (res.has_value()) {
        m_levels = res.value().at(difficulty);
        m_loadCallback(Ok(false));
        this->loadDuel(0, TransitionMode::Instant);
        return;
    }

    m_loadCallback(Ok(true));

    all.requestAnchorLevels([this, selfref = WeakRef(this), difficulty](Result<const std::unordered_map<Difficulty, std::vector<LevelRanking>>&> res) {
        if (!selfref.lock()) {
            return;
        }

        if (!res.isOk()) {
            m_loadCallback(Err(""));
            return;
        }

        m_loadCallback(Ok(false));
        m_levels = res.unwrap().at(difficulty);

        std::erase_if(m_levels, [this](const LevelRanking& level) {
            return level.id == m_levelID;
        });

        this->loadDuel(0, TransitionMode::Instant);
    });
}

bool LevelVersusLayer::onBack() {
    if (m_currentDuel <= 0 || m_duelHistory.empty()) {
        return true;
    }

    if (!m_animating) {
        this->loadDuel(m_currentDuel - 1, m_duelHistory.back().choseHarder ? TransitionMode::Up : TransitionMode::Down);
    }

    return false;
}