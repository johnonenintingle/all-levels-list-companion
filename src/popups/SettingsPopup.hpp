#pragma once

#include "Includes.hpp"

class SettingsPopup : public Popup {

private:

    bool init() override;

public:

    static SettingsPopup* create();

};