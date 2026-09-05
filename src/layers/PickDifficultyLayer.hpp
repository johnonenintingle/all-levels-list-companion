#pragma once

#include "Includes.hpp"

class DifficultyNode;

class PickDifficultyLayer : public CCNode {

private:

    CCLabelBMFont* m_listNameLabel = nullptr;

    Function<void(Result<>)> m_loadedCallback;
    Function<void(int, Difficulty, std::vector<LevelRanking>)> m_selectedCallback;

    std::vector<UserList> m_lists;

    std::vector<DifficultyNode*> m_difficultyNodes;

    int m_levelID;
    int m_listIndex = 0;

    bool m_isLoading = true;
    bool m_isSwitching = false;

    PickDifficultyLayer(int, Function<void(Result<>)>, Function<void(int, Difficulty, std::vector<LevelRanking>)>);

    bool init() override;

    void update(float) override;

    void switchToList(int);
    void loadLevelCounts();
    void selectList(Difficulty);

public:

    static PickDifficultyLayer* create(int, Function<void(Result<>)>, Function<void(int, Difficulty, std::vector<LevelRanking>)>);

};