#pragma once

#include "Includes.hpp"

class ALLLevelCell : public CCNode {

private:

    bool init() override;

public:

    static ALLLevelCell* create();

};