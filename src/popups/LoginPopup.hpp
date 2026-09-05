#pragma once

#include "Includes.hpp"
#include "LoaderPopup.hpp"

class LoadingScreenLayer;

class LoginPopup : public LoaderPopup {

private:

    NineSlice* m_websiteBg = nullptr;
    CCLabelBMFont* m_errorLbl = nullptr;

    bool m_hoveringWebsite = false;

    bool init() override;

    void onLogin(Result<>);
    void setError(ZStringView);

    void update(float) override;

    ZStringView getLoadingText() override {
        return "Logging in...";
    }

public:

    static LoginPopup* create();

};