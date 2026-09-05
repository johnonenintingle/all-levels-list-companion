#pragma once

#include "Includes.hpp"

class UnderlinedLabel : public CCNodeRGBA {

private:

    CCLabelBMFont* m_label = nullptr;
    NineSlice* m_underline = nullptr;

    bool init(ZStringView, ZStringView);

public:

    static UnderlinedLabel* create(ZStringView, ZStringView);

    void setOpacity(GLubyte) override;

};