#pragma once

#include "Includes.hpp"

class ProScrollLayer;

class AreaTextInput : public CCNode {

private:

    Function<void(const std::string&)> m_callback;

    ProScrollLayer* m_scroll = nullptr;
    TextInput* m_input = nullptr;
    CCNode* m_container = nullptr;
    CCLabelBMFont* m_cursor = nullptr;
    CCLabelBMFont* m_placeholder = nullptr;

    std::string m_text;

    int m_cursorPos = -1;
    int m_opacity = 215;

    bool m_enabled = true;

    float m_scale = 1.f;

    AreaTextInput(Function<void(const std::string&)>);

    bool init(const CCSize&);

    void checkLabel(std::string, CCLabelBMFont*);
    CCLabelBMFont* getLastLine();
    CCPoint getCursorPos(int);
    int cursorPosForClick(const CCPoint&);

    void update(float) override;
    
public:

    static AreaTextInput* create(const CCSize&, Function<void(const std::string&)>);

    void setText(const std::string&);

    float getScrollPosition();
    void scrollToTop();
    void setEnabled(bool);
    void setOpacity(int);

    void setScale(float) override;
    float getScale() override;

};