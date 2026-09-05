#pragma once

#include "Includes.hpp"
#include "ALLManager.hpp"
#include "LoaderPopup.hpp"

#include <Geode/ui/Button.hpp>

class AreaTextInput;
class RatingSlider;

class RateLevelPopup : public LoaderPopup {

private:

    CCLabelBMFont* m_ratingLabel = nullptr;
    RatingSlider* m_ratingSlider = nullptr;
    AreaTextInput* m_areaInput;
    CCLayerGradient* m_inputGradient = nullptr;
    CCLabelBMFont* m_lengthLabel = nullptr;
    Button* m_clearRatingBtn = nullptr;
    Button* m_clearReviewBtn = nullptr;
    Button* m_submitBtn = nullptr;

    float m_currentValue = 0.f;

    bool m_fetching = false;
    bool m_rated = false;

    int m_levelID;

    LevelRating m_originalRating;
    LevelRating m_currentRating;

    RateLevelPopup(int);

    bool init(std::string_view);

    void loadRating();

    void updateButtons();

    void update(float) override;

    ZStringView getLoadingText() override {
        return m_fetching ? "Fetching..." : "Submitting...";
    }

public:

    static RateLevelPopup* create(GJGameLevel*);

};