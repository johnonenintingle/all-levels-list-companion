#pragma once

#include "Includes.hpp"

class LoadingScreenLayer;

class LoaderPopup : public Popup {

private:

    LoadingScreenLayer* m_loadingLayer = nullptr;

protected:

    bool m_isLoading = false;

    void setLoading(bool);

    void updateLoadingText();

    virtual ZStringView getLoadingText() {
        return "Loading...";
    }

};