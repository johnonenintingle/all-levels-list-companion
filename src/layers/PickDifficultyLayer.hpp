#pragma once

#include "Includes.hpp"

class DifficultyNode;

class PickDifficultyLayer : public CCNode {

private:

    CCLabelBMFont* m_listNameLabel = nullptr;

    Function<void(Result<>)> m_loadedCallback;
    Function<void(int64_t, Difficulty, std::vector<LevelRanking>)> m_selectedCallback;

    std::vector<UserList> m_lists;

    std::vector<DifficultyNode*> m_difficultyNodes;

    int64_t m_levelID;
    int m_listIndex = 0;

    bool m_isLoading = true;
    bool m_isSwitching = false;

    PickDifficultyLayer(int64_t, Function<void(Result<>)>, Function<void(int64_t, Difficulty, std::vector<LevelRanking>)>);

    bool init() override;

    void update(float) override;

    void switchToList(int);
    void loadLevelCounts();
    void selectList(Difficulty);

public:

    static PickDifficultyLayer* create(int64_t, Function<void(Result<>)>, Function<void(int64_t, Difficulty, std::vector<LevelRanking>)>);

};