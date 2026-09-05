#pragma once

#include "Includes.hpp"

class DifficultyNode : public CCNode {

private:

    NineSlice* m_bg = nullptr;

    CCLabelBMFont* m_countLabel = nullptr;
    
    CCLayerGradient* m_gradient = nullptr;

    Difficulty m_difficulty;

    bool m_hovered = false;

    DifficultyNode(Difficulty);

    bool init() override;

public:

    static DifficultyNode* create(Difficulty);

    Difficulty getDifficulty();

    void setCount(int);

    void setHovered(bool);

};