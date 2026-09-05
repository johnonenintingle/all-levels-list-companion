#pragma once

#include "Includes.hpp"
#include "LoaderPopup.hpp"

#include <Geode/ui/Button.hpp>

class ProfilePopup : public LoaderPopup {

private:

    CCLabelBMFont* m_pendingLabel = nullptr;
    Button* m_syncPendingButton = nullptr;

    bool init() override;

    void updatePendingCompletions();

    ZStringView getLoadingText() override {
        return "Syncing...";
    }

public:

    static ProfilePopup* create();

};