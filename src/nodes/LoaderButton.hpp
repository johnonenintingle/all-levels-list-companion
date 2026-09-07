#pragma once

#include "Includes.hpp"

#include <Geode/ui/Button.hpp>

class LoaderButton : public CCNode {

private:

    Function<void()> m_callback;

    Button* m_button = nullptr;
    CCSprite* m_loadingCircle = nullptr;
    CCSprite* m_plainSprite = nullptr;
    CCSpriteGrayscale* m_grayscaleSprite = nullptr;
    CCSpriteGrayscale* m_grayscalePlainSprite = nullptr;

    std::string m_errorString;

    bool m_loading = false;
    bool m_error = false;

    LoaderButton(Function<void()>);

    bool init(ZStringView, float, CCPoint);

public:

    static LoaderButton* create(ZStringView, float, CCPoint, Function<void()>);

    void setLoading(bool);
    void setError(std::string);

};