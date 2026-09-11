#pragma once

#include "Includes.hpp"

class ALLListLayer : public CCLayer {

private:

    bool init() override;

    void keyBackClicked() override;

public:

    static ALLListLayer* create();
    static CCScene* scene();

};