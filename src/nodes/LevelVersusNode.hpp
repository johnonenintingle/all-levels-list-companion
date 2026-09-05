#pragma once

#include "Includes.hpp"

class LevelVersusNode : public CCNode {

public:

    enum class Mode {
        Top,
        Bottom,
        Main
    };

private:

    CCLayerColor* m_overlay = nullptr;
    CCLayerGradient* m_gradient = nullptr;
    CCSprite* m_sprite = nullptr;
    CCLabelBMFont* m_nameLabel = nullptr;
    NineSlice* m_border = nullptr;
    NineSlice* m_bg = nullptr;
    CCLayerGradient* m_hoverGradient = nullptr;

    Mode m_mode = Mode::Main;

    bool m_hovering = false;
    bool m_animating = false;

    float m_ogLabelScale = 1.f;

    bool init();
    bool init(const LevelRanking&);

public:

    static LevelVersusNode* create();
    static LevelVersusNode* create(const LevelRanking&);

    void setLevel(const LevelRanking&, bool = false, bool = false);
    void setHovering(bool);
    void setMode(Mode, bool);
    void setHidden(bool, bool);

};