#pragma once

#include "Includes.hpp"

class RankResultLayer : public CCNode {

private:

    Function<void(Result<bool>)> m_loadCallback;

    GJGameLevel* m_level = nullptr;
    CCNode* m_container = nullptr;
    
    std::vector<LevelRanking> m_levels;

    Difficulty m_difficulty;

    int m_placement = 0;
    int64_t m_refAbove = 0;
    int64_t m_refBelow = 0;
    int m_listID = 0;

    RankResultLayer(GJGameLevel*, Function<void(Result<bool>)>);

    bool init() override;

    void loadFor(int64_t, bool, int64_t, int64_t, int, Difficulty, std::vector<LevelRanking>);

public:

    static RankResultLayer* create(GJGameLevel*, Function<void(Result<bool>)>);

    void setLevel(int64_t, bool, int64_t, int64_t, int, Difficulty);
    int getPlacement();

    void submitPlacement(Function<void(Result<>)>);

};