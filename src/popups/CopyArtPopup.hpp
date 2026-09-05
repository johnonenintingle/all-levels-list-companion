#pragma once

#include "Includes.hpp"

class CopyArtPopup : public Popup {

private:

    bool init(std::string);

public:

    static CopyArtPopup* create(std::string);

};