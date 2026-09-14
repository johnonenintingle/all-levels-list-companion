#pragma once

#include "Includes.hpp"

class LevelVersusNode;

class LevelVersusLayer : public CCNode {

private:

    enum class TransitionMode {
        Instant,
        Up,
        Down
    };

    struct Duel {
        bool choseHarder;
        int64_t refAbove;
        int64_t refBelow;
    };

    Function<void(Result<bool>)> m_loadCallback;
    Function<void(int64_t, bool, int64_t, int64_t, int, Difficulty)> m_finishCallback;

    CCLabelBMFont* m_duelLabel = nullptr;
    CCLayerGradient* m_gradientBar = nullptr;
    NineSlice* m_barStencil = nullptr;

    LevelVersusNode* m_mainNode = nullptr;
    CCNode* m_nodesContainer = nullptr;

    std::array<LevelVersusNode*, 4> m_nodes = { nullptr, nullptr, nullptr, nullptr };
    std::vector<Duel> m_duelHistory;

    std::vector<LevelRanking> m_levels;

    Difficulty m_difficulty;

    int64_t m_levelID;
    int m_listID;
    int m_currentDuel = -1;
    int m_lastIndex = -1;

    int64_t m_refAbove = 0;
    int64_t m_refBelow = 0;
    
    bool m_animating = false;
    bool m_usingRefs = false;

    LevelVersusLayer(Function<void(Result<bool>)>, Function<void(int64_t, bool, int64_t, int64_t, int, Difficulty)>);

    bool init(GJGameLevel*);

    void loadDuel(int, TransitionMode);

    void update(float) override;

public:

    static LevelVersusLayer* create(GJGameLevel*, Function<void(Result<bool>)>, Function<void(int64_t, bool, int64_t, int64_t, int, Difficulty)>);

    void setLevels(int, Difficulty, std::vector<LevelRanking>);

    bool onBack();

};