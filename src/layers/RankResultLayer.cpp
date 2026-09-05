#include "RankResultLayer.hpp"
#include "ALLManager.hpp"

#include <Geode/ui/Button.hpp>

RankResultLayer::RankResultLayer(GJGameLevel* level, Function<void(Result<bool>)> loadCallback)
    : m_level(level), m_loadCallback(std::move(loadCallback)) {}

RankResultLayer* RankResultLayer::create(GJGameLevel* level, Function<void(Result<bool>)> loadCallback) {
    auto ret = new RankResultLayer(level, std::move(loadCallback));

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool RankResultLayer::init() {
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({260, 185});
    
    return true;
};

void RankResultLayer::setLevel(int id, bool harder, int refAbove, int refBelow, int listID, Difficulty difficulty) {
    auto& all = ALLManager::get();
    
    if (!all.areUserListsCached()) {
        m_loadCallback(Ok(true));
    }

    all.getUserLists([this, selfref = WeakRef(this), id, harder, refAbove, refBelow, listID, difficulty](Result<const std::vector<UserList>&> res) {
        if (!selfref.lock()) {
            return;
        }

        if (!res.isOk()) {
            m_loadCallback(Err(""));
            return;
        }

        m_loadCallback(Ok(false));

        this->loadFor(id, harder, refAbove, refBelow, listID, difficulty, res.unwrap().front().levels);
    });
}

void RankResultLayer::loadFor(int id, bool harder, int refAbove, int refBelow, int listID, Difficulty difficulty, std::vector<LevelRanking> levels) {
    m_levels = std::move(levels);
    m_difficulty = difficulty;
    m_refAbove = refAbove;
    m_refBelow = refBelow;
    m_listID = listID;

    std::erase_if(m_levels, [this](const LevelRanking& level) {
        return level.id == m_level->m_levelID.value();
    });

    auto found = false;
    auto index = -1;

    for (const auto& level : m_levels) {
        index++;

        if (level.id == id) {
            found = true;
            break;
        }
    }

    if (!found) {
        index = 0;

        for (const auto& level : m_levels) {
            index++;
            
            if (level.bucket < difficulty) {
                index--;
                break;
            }
        }
    } else if (!harder) {
        index++;
    }

    m_placement = index;

    if (m_container) {
        m_container->removeFromParent();
    }

    m_container = CCNode::create();
    m_container->setAnchorPoint({0.5f, 0.5f});
    m_container->setLayout(
        SimpleAxisLayout::create(Axis::Column)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setGap(8.f)
    );
    m_container->setPosition(this->getContentSize() / 2.f);

    this->addChild(m_container);

    const auto createNode = [this, index](ZStringView text, Difficulty difficulty, int mode, bool secondary, bool harder = false) {
        auto bg = NineSlice::create("square02b_001.png");
        bg->setScale(0.7f);
        bg->setContentSize(CCSize{this->getContentWidth() - 20.f, secondary ? 30.f : 50.f} / bg->getScale());
        bg->setColor({0, 0, 0});
        bg->setOpacity(secondary ? 40 : 65);
        
        m_container->addChild(bg);

        if (mode != 1) {
            auto lbl = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
            lbl->setOpacity(135);
            lbl->setScale(0.35f);
            lbl->setPosition(bg->getContentSize() / 2.f);

            bg->addChild(lbl);

            return;
        }

        auto sub = CCNode::create();
        sub->setAnchorPoint({0, 0.5f});
        sub->setPosition({secondary ? 24.f : 18.f, bg->getContentHeight() / 2.f});
        sub->setLayout(
            SimpleAxisLayout::create(Axis::Row)
                ->setMainAxisScaling(AxisScaling::Grow)
                ->setMainAxisAlignment(MainAxisAlignment::Start)
                ->setCrossAxisScaling(AxisScaling::Grow)
                ->setGap(8.f)
        );

        bg->addChild(sub);

        auto spr = iconForDifficulty(difficulty);
        spr->setScale(secondary ? 0.6f : 1.f);
        spr->setOpacity(secondary ? 210 : 255);

        sub->addChild(spr);

        auto lbl = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        lbl->setOpacity(secondary ? 135 : 200);
        lbl->limitLabelWidth(this->getContentWidth() - 40.f, secondary ? 0.35f : 0.5f, 0.f);

        sub->addChild(lbl);

        sub->updateLayout();

        if (secondary) {
            lbl = CCLabelBMFont::create(harder ? "Harder" : "Easier", "bigFont.fnt");
            lbl->setOpacity(120);
            lbl->setScale(0.32f);
            lbl->setPosition({bg->getContentWidth() - 18, bg->getContentHeight() / 2.f});
            lbl->setAnchorPoint({1, 0.5f});

            bg->addChild(lbl);
        } else {
            sub = CCNode::create();
            sub->setAnchorPoint({1, 0.5f});
            sub->setPosition({bg->getContentWidth() - 18, bg->getContentHeight() / 2.f});
            sub->setLayout(
                SimpleAxisLayout::create(Axis::Row)
                    ->setMainAxisScaling(AxisScaling::Grow)
                    ->setMainAxisAlignment(MainAxisAlignment::Start)
                    ->setCrossAxisScaling(AxisScaling::Grow)
                    ->setGap(2.2f)
            );

            bg->addChild(sub);

            spr = CCSprite::create("hashtag.png"_spr);
            spr->setScale(0.175f);
            spr->setOpacity(200);
            
            sub->addChild(spr);

            lbl = CCLabelBMFont::create(numToString(index + 1).c_str(), "bigFont.fnt");
            lbl->setScale(0.675f);
            lbl->setOpacity(200);
            
            sub->addChild(lbl);

            sub->updateLayout();

            lbl->setPositionY(lbl->getPositionY() + 1);
        }
    };

    if (index <= 0) {
        createNode("Top of the ranking", Difficulty::Unknown, 2, true, true);
    } else {
        createNode(m_levels[index - 1].name, m_levels[index - 1].bucket, 1, true, true);
    }

    auto line = NineSlice::create("square02b_001.png");
    line->setScale(0.2f);
    line->setContentSize(CCSize{this->getContentWidth() - 35.f, 2.5f} / line->getScale());
    line->setOpacity(25);
    
    m_container->addChild(line);

    std::string levelName = m_level->m_levelName;

    createNode(levelName, difficultyForLevel(m_level), 1, false);

    line = NineSlice::create("square02b_001.png");
    line->setScale(0.2f);
    line->setContentSize(CCSize{this->getContentWidth() - 35.f, 2.5f} / line->getScale());
    line->setOpacity(25);
    
    m_container->addChild(line);

    if (index >= static_cast<int>(m_levels.size())) {
        createNode("Bottom of the ranking", Difficulty::Unknown, 2, true, false);
    } else {
        createNode(m_levels[index].name, m_levels[index].bucket, 1, true, false);
    }

    m_container->updateLayout();
}

int RankResultLayer::getPlacement() {
    return m_placement + 1;
}

void RankResultLayer::submitPlacement(Function<void(Result<>)> placeCallback) {
    auto infos = std::vector<LevelSubmitInfo>{};

    for (const auto& level : m_levels) {
        infos.push_back(LevelSubmitInfo{
            .id = level.id,
            .bucket = Difficulty::Unknown,
            .refAbove = 0,
            .refBelow = 0,
            .listID = level.listID == m_listID ? m_listID : 0
        });
    }

    if (m_placement >= static_cast<int>(infos.size())) {
        infos.push_back(LevelSubmitInfo{
            .id = m_level->m_levelID.value(),
            .bucket = m_difficulty,
            .refAbove = m_refAbove,
            .refBelow = m_refBelow,
            .listID = m_listID
        });

    } else {
        infos.insert(infos.begin() + m_placement, LevelSubmitInfo{
            .id = m_level->m_levelID.value(),
            .bucket = m_difficulty,
            .refAbove = m_refAbove,
            .refBelow = m_refBelow,
            .listID = m_listID
        });
    }

    ALLManager::get().submitDifficultyPlacements(std::move(infos), [this, callback = std::move(placeCallback)](Result<> res) mutable {
        callback(res);
    });
}