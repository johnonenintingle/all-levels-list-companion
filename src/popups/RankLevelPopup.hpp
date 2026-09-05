#pragma once

#include "Includes.hpp"
#include "LoaderPopup.hpp"

#include <Geode/ui/Button.hpp>

class PickDifficultyLayer;
class LevelVersusLayer;
class RankResultLayer;

class RankLevelPopup : public LoaderPopup {

private:

    CCLabelBMFont* m_instructionLabel = nullptr;
    PickDifficultyLayer* m_pickDifficultyLayer = nullptr;
    RankResultLayer* m_rankResultLayer = nullptr;
    LevelVersusLayer* m_levelVersusLayer = nullptr;
    Button* m_backButton = nullptr;
    Button* m_placeButton = nullptr;
    NineSlice* m_overlay = nullptr;

    std::string m_levelName;

    int m_levelID;
    int m_currentStep = 0;
    int m_placement = 0;

    bool m_submitting = false;

    RankLevelPopup(int, std::string);

    bool init(GJGameLevel*);

    void updateStep();
    void onBack();

    void keyDown(enumKeyCodes, double) override;

    ZStringView getLoadingText() override {
        return m_submitting ? "Submitting..." : "Fetching...";
    }

public:

    static RankLevelPopup* create(GJGameLevel*);

};