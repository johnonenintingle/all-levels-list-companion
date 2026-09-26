#pragma once

#include "Includes.hpp"

class CopyArtPopup : public Popup {

private:

    bool init(LevelEditorLayer*);

public:

    static CopyArtPopup* create(LevelEditorLayer*);

};