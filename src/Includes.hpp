#pragma once

using namespace geode::prelude;

const int REVIEW_CHARACTER_LIMIT = 2000;

enum class Difficulty {
    Impossible = 11,
    ExtremeDemon = 10,
    InsaneDemon = 9,
    HardDemon = 6,
    MediumDemon = 8,
    EasyDemon = 7,
    Insane = 5,
    Harder = 4,
    Hard = 3,
    Normal = 2,
    Easy = 1,
    Auto = 0,
    Unknown = -1
};

constexpr int difficultyRank(Difficulty difficulty) {
    switch (difficulty) {
        default:
        case Difficulty::Unknown: return 0;
        case Difficulty::Auto: return 1;
        case Difficulty::Easy: return 2;
        case Difficulty::Normal: return 3;
        case Difficulty::Hard: return 4;
        case Difficulty::Harder: return 5;
        case Difficulty::Insane: return 6;
        case Difficulty::EasyDemon: return 7;
        case Difficulty::MediumDemon: return 8;
        case Difficulty::HardDemon: return 9;
        case Difficulty::InsaneDemon: return 10;
        case Difficulty::ExtremeDemon: return 11;
        case Difficulty::Impossible: return 12;
    }
}

constexpr bool operator<(Difficulty a, Difficulty b) {
    return difficultyRank(a) < difficultyRank(b);
}

constexpr bool operator>(Difficulty a, Difficulty b) {
    return difficultyRank(a) > difficultyRank(b);
}

constexpr bool operator<=(Difficulty a, Difficulty b) {
    return difficultyRank(a) <= difficultyRank(b);
}

constexpr bool operator>=(Difficulty a, Difficulty b) {
    return difficultyRank(a) >= difficultyRank(b);
}

struct UserInfo {
    int id = 0;
    std::string username = "";
    std::string avatarUrl = "";
};

struct LevelRating {
    float rating = 0.f;
    bool rated = false;
    std::string review = "";
    
    bool operator==(const LevelRating& other) const {
        return rated == other.rated && ((rating == other.rating) || (!rated)) && review == other.review;
    };
};

struct LevelRanking {
    int64_t id;
    std::string name;
    int listID;
    Difficulty bucket = Difficulty::Unknown;
};

struct UserList {
    int id = 0;
    std::string name;
    int sortOrder;
    std::vector<LevelRanking> levels;
};

static ccColor3B colorForDifficulty(Difficulty difficulty) {
    switch (difficulty) {
        default: return ccColor3B{ 153, 153, 153 };
        case Difficulty::Auto: return ccColor3B{ 255, 244, 114 };
        case Difficulty::Easy: return ccColor3B{ 0, 212, 255 };
        case Difficulty::Normal: return ccColor3B{ 0, 255, 54 };
        case Difficulty::Hard: return ccColor3B{ 255, 231, 0 };
        case Difficulty::Harder: return ccColor3B{ 255, 114, 0 };
        case Difficulty::Insane: return ccColor3B{ 255, 120, 216 };
        case Difficulty::EasyDemon: return ccColor3B{ 148, 71, 249 };
        case Difficulty::MediumDemon: return ccColor3B{ 216, 56, 205 };
        case Difficulty::HardDemon: return ccColor3B{ 255, 56, 78 };
        case Difficulty::InsaneDemon: return ccColor3B{ 245, 34, 28 };
        case Difficulty::ExtremeDemon: return ccColor3B{ 184, 0, 0 };
        case Difficulty::Impossible: return ccColor3B{ 228, 126, 23 };
    }
}

static Difficulty difficultyForString(std::string_view str) {
    if (str == "Impossible") { return Difficulty::Impossible; }
    else if (str == "Extreme Demon") { return Difficulty::ExtremeDemon; }
    else if (str == "Insane Demon") { return Difficulty::InsaneDemon; }
    else if (str == "Hard Demon") { return Difficulty::HardDemon; }
    else if (str == "Medium Demon") { return Difficulty::MediumDemon; }
    else if (str == "Easy Demon") { return Difficulty::EasyDemon; }
    else if (str == "Insane") { return Difficulty::Insane; }
    else if (str == "Harder") { return Difficulty::Harder; }
    else if (str == "Hard") { return Difficulty::Hard; }
    else if (str == "Normal") { return Difficulty::Normal; }
    else if (str == "Easy") { return Difficulty::Easy; }
    else if (str == "Auto") { return Difficulty::Auto; }
    else { return Difficulty::Unknown; }
}

static std::string_view stringForDifficulty(Difficulty difficulty) {
    switch (difficulty) {
        default:
        case Difficulty::Unknown: return "NA";
        case Difficulty::Impossible: return "Impossible";
        case Difficulty::ExtremeDemon: return "Extreme Demon";
        case Difficulty::InsaneDemon: return "Insane Demon";
        case Difficulty::HardDemon: return "Hard Demon";
        case Difficulty::MediumDemon: return "Medium Demon";
        case Difficulty::EasyDemon: return "Easy Demon";
        case Difficulty::Insane: return "Insane";
        case Difficulty::Harder: return "Harder";
        case Difficulty::Hard: return "Hard";
        case Difficulty::Normal: return "Normal";
        case Difficulty::Easy: return "Easy";
        case Difficulty::Auto: return "Auto";
    }
}

static std::string_view stringForLength(int length) {
    switch (length) {
        default: return "";
        case 0: return "Tiny";
        case 1: return "Short";
        case 2: return "Medium";
        case 3: return "Long";
        case 4: return "XL";
        case 5: return "Platformer";
    }
}

static Difficulty difficultyForLevel(GJGameLevel* level) {
    int difficulty;

    if (level->m_demon > 0) {
        difficulty = level->m_demonDifficulty > 0 ? level->m_demonDifficulty + 4 : 6;
    } else if (level->m_autoLevel) {
        difficulty = -1;
    } else if (level->m_ratings < 5) {
        difficulty = 0;
    } else {
        difficulty = level->m_ratingsSum / level->m_ratings;
    }

    if (difficulty == -1) {
        return Difficulty::Auto;
    } else if (difficulty <= 0) {
        return Difficulty::Unknown;
    }

    return static_cast<Difficulty>(difficulty);
}

static CCSprite* iconForDifficulty(Difficulty difficulty) {
    if (difficulty == Difficulty::Impossible) {
        return CCSprite::create("impossible_demon_2.png"_spr);
    } else {
        return CCSprite::createWithSpriteFrameName(
            difficulty == Difficulty::Auto
                ? "diffIcon_auto_btn_001.png"
                : fmt::format("diffIcon_{:02}_btn_001.png", difficulty == Difficulty::Unknown ? 0 : static_cast<int>(difficulty)).c_str()
        );
    }
}

static bool isHoveringNode(const CCPoint& pos, CCNode* node) {
    auto localPos = node->convertToNodeSpace(pos);
    auto rect = CCRect{0, 0, node->getContentSize().width, node->getContentSize().height};
    return rect.containsPoint(localPos);
}

static std::string getBaseURL() {
    auto url = Mod::get()->getSavedValue<std::string>("api-base-url");

    while (std::string_view(url).ends_with("/")) {
        url.pop_back();
    }

    return url;
}

static std::vector<GJGameLevel*> getCompletedLevels() {
    auto ret = std::vector<GJGameLevel*>{};

    for (const auto& [_, level] : CCDictionaryExt<const char*, GJGameLevel*>(GameLevelManager::get()->m_onlineLevels)) {
        if (level->m_normalPercent.value() >= 100) {
            ret.push_back(level);
        }
    }

    return ret;
}