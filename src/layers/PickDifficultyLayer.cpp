#include "PickDifficultyLayer.hpp"
#include "ALLManager.hpp"

#include "nodes/DifficultyNode.hpp"

#include <Geode/ui/Button.hpp>

PickDifficultyLayer::PickDifficultyLayer(int levelID, Function<void(Result<>)> loadedCallback, Function<void(int, Difficulty, std::vector<LevelRanking>)> selectedCallback)
    : m_levelID(levelID), m_loadedCallback(std::move(loadedCallback)), m_selectedCallback(std::move(selectedCallback)) {}

PickDifficultyLayer* PickDifficultyLayer::create(int levelID, Function<void(Result<>)> loadedCallback, Function<void(int, Difficulty, std::vector<LevelRanking>)> selectedCallback) {
    auto ret = new PickDifficultyLayer(levelID, std::move(loadedCallback), std::move(selectedCallback));

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool PickDifficultyLayer::init() {
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({260, 185});

    auto diffContainer = CCNode::create();
    diffContainer->setContentWidth(234);
    diffContainer->setAnchorPoint({0.5f, 1});
    diffContainer->setPosition({this->getContentWidth() / 2.f, 174});
    diffContainer->setLayout(
        AxisLayout::create()
            ->setGrowCrossAxis(true)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisAlignment(AxisAlignment::End)
            ->setAutoScale(false)
            ->setGap(4.7f)
            ->ignoreInvisibleChildren(false)
    );

    this->addChild(diffContainer);

    const auto makeDifficultyButton = [this, diffContainer](Difficulty difficulty) {
        auto node = DifficultyNode::create(difficulty);
        m_difficultyNodes.push_back(node);
        
        auto btn = Button::createWithNode(node, [this, difficulty](Button*) {
            this->selectList(difficulty);
        });
        btn->setScaleMultiplier(1.05f);

        diffContainer->addChild(btn);
    };

    makeDifficultyButton(Difficulty::Impossible);
    makeDifficultyButton(Difficulty::ExtremeDemon);
    makeDifficultyButton(Difficulty::InsaneDemon);
    makeDifficultyButton(Difficulty::HardDemon);
    makeDifficultyButton(Difficulty::MediumDemon);
    makeDifficultyButton(Difficulty::EasyDemon);
    makeDifficultyButton(Difficulty::Insane);
    makeDifficultyButton(Difficulty::Harder);
    makeDifficultyButton(Difficulty::Hard);
    makeDifficultyButton(Difficulty::Normal);
    makeDifficultyButton(Difficulty::Easy);
    makeDifficultyButton(Difficulty::Auto);

    diffContainer->updateLayout();
    
    auto btnSpr = ButtonSprite::create("<");
    btnSpr->m_label->setPositionY(btnSpr->m_label->getPositionY() + 1.5f);

    auto btn = Button::createWithNode(btnSpr, [this](Button*) {
        this->switchToList(m_listIndex - 1);
    });
    btn->setScaleMultiplier(1.1f);
    btn->setScale(0.675f);
    btn->setPosition({this->getContentWidth() / 2.f - 50.f, 18});

    this->addChild(btn, 1);

    btnSpr = ButtonSprite::create(">");
    btnSpr->m_label->setPosition(btnSpr->m_label->getPosition() + CCPoint{1, 1.5f});

    btn = Button::createWithNode(btnSpr, [this](Button*) {
        this->switchToList(m_listIndex + 1);
    });
    btn->setScaleMultiplier(1.1f);
    btn->setScale(0.675f);
    btn->setPosition({this->getContentWidth() / 2.f + 50.f, 18});

    this->addChild(btn, 1);

    auto stencil = CCLayerColor::create({255, 255, 255, 255}, 100, 40);
    stencil->ignoreAnchorPointForPosition(false);
    stencil->setPosition({this->getContentWidth() / 2.f, 18.8f});

    auto clip = CCClippingNode::create();
    clip->setStencil(stencil);

    this->addChild(clip);

    m_listNameLabel = CCLabelBMFont::create("All", "bigFont.fnt");
    m_listNameLabel->setPosition({this->getContentWidth() / 2.f, 18.8f});
    m_listNameLabel->setScale(0.44f);
    m_listNameLabel->setOpacity(230);

    clip->addChild(m_listNameLabel);

    ALLManager::get().getUserLists([selfref = WeakRef(this)](Result<const std::vector<UserList>&> res) {
        auto self = selfref.lock();

        if (!self) {
            return;
        }

        self->m_isLoading = false;

        if (!res.isOk()) {
            self->m_loadedCallback(Err(""));
            return;
        }

        self->m_loadedCallback(Ok());
        self->m_lists = res.unwrap();
        self->loadLevelCounts();
    });    

    #ifndef GEODE_IS_MOBILE

    this->scheduleUpdate();
    
    #endif

    return true;
}

void PickDifficultyLayer::update(float) {
    auto mousePos = getMousePos();

    for (auto node : m_difficultyNodes) {
        node->setHovered(!m_isLoading && isHoveringNode(mousePos, node));
    }
}

void PickDifficultyLayer::switchToList(int index) {
    if (m_isSwitching || m_lists.empty()) {
        return;
    }

    if (index == m_listIndex) {
        this->loadLevelCounts();
        return;
    }

    auto left = index - m_listIndex > 0;
    
    m_listIndex = index;

    this->loadLevelCounts();

    m_isSwitching = true;

    m_listNameLabel->stopAllActions();
    m_listNameLabel->runAction(CCSequence::create(
        CCEaseSineIn::create(CCMoveTo::create(0.09f, {
            this->getContentWidth() / 2.f + (50.f * (left ? -1 : 1)) + (m_listNameLabel->getScaledContentWidth() / 2.f * ((left ? -1 : 1))),
            m_listNameLabel->getPositionY()
        })),
        CCDelayTime::create(0.05f),
        CallFuncExt::create([this, left] {
            m_listNameLabel->stopAllActions();
            m_listNameLabel->setString(m_lists[m_listIndex].name.c_str());
            m_listNameLabel->setPositionX(this->getContentWidth() / 2.f + (50.f * (left ? 1 : -1)) + (m_listNameLabel->getScaledContentWidth() / 2.f * ((left ? 1 : -1))));
            m_listNameLabel->runAction(CCSequence::create(
                CCEaseSineOut::create(CCMoveTo::create(0.09f, {
                    this->getContentWidth() / 2.f,
                    m_listNameLabel->getPositionY()
                })),
                CallFuncExt::create([this] {
                    m_isSwitching = false;
                }),
                nullptr
            ));
        }),
        nullptr
    ));
}

void PickDifficultyLayer::loadLevelCounts() {
    if (m_lists.empty()) {
        return;
    }

    if (m_listIndex >= static_cast<int>(m_lists.size())) {
        m_listIndex = 0;
    }

    if (m_listIndex < 0) {
        m_listIndex = static_cast<int>(m_lists.size()) - 1;
    }

    auto counts = std::unordered_map<Difficulty, int>{};

    for (const auto& level : m_lists[m_listIndex].levels) {
        if (level.id > 0 && level.id != m_levelID) {
            counts[level.bucket]++;
        }
    }

    for (auto node : m_difficultyNodes) {
        node->setCount(counts[node->getDifficulty()]);
    }
}

void PickDifficultyLayer::selectList(Difficulty difficulty) {
    auto levels = std::vector<LevelRanking>{};

    for (const auto& level : m_lists[m_listIndex].levels) {
        if (level.bucket == difficulty) {
            levels.push_back(level);
        }
    }

    m_selectedCallback(m_lists[m_listIndex].id, difficulty, std::move(levels));
}