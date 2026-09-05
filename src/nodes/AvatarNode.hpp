#pragma once

#include "Includes.hpp"

class AvatarNode : public CCNode {

private:

    bool init(const std::string&);

public:

    static AvatarNode* create(const std::string&);

};