#pragma once

#include "Includes.hpp"

class LoadingScreenLayer : public CCLayer {

private:

    CCLabelBMFont* m_label = nullptr;
    Ref<CCNode> m_closeBtn = nullptr;

    ~LoadingScreenLayer();
    LoadingScreenLayer(CCNode*);

    bool init(ZStringView, const CCSize&);

    void registerWithTouchDispatcher() override;

    bool ccTouchBegan(CCTouch*, CCEvent*) override;

public:

    static LoadingScreenLayer* create(ZStringView, const CCSize&, CCNode*);

    void setLoadingText(ZStringView);

};