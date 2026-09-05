#include "LoaderPopup.hpp"

#include "layers/LoadingScreenLayer.hpp"

void LoaderPopup::setLoading(bool loading) {
    if (m_isLoading == loading) {
        return;
    }

    m_isLoading = loading;
    
    if (loading && m_loadingLayer) {
        m_loadingLayer->setVisible(true);
        this->updateLoadingText();
        return;
    }

    if (!loading && m_loadingLayer) {
        m_loadingLayer->setVisible(false);
    }

    if (!loading) {
        return;
    }

    m_loadingLayer = LoadingScreenLayer::create(this->getLoadingText(), m_size, m_closeBtn);
    
    m_mainLayer->addChild(m_loadingLayer, 10);
}

void LoaderPopup::updateLoadingText() {
    if (m_loadingLayer) {
        m_loadingLayer->setLoadingText(this->getLoadingText());
    }
}