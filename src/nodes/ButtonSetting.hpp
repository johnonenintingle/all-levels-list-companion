#pragma once

#include "Includes.hpp"

#include "popups/SettingsPopup.hpp"

#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>

namespace {

class ButtonSetting : public SettingV3 {

public:

    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
        auto res = std::make_shared<ButtonSetting>();
        auto root = checkJson(json, "ButtonSetting");

        res->init(key, modID, root);
        res->parseNameAndDescription(root);
        res->parseEnableIf(root);

        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    bool load(matjson::Value const& json) override {
        return true;
    }
    bool save(matjson::Value& json) const override {
        return true;
    }

    bool isDefaultValue() const override {
        return true;
    }
    void reset() override {}

    SettingNodeV3* createNode(float width) override;
};

class ButtonSettingNode : public SettingNodeV3 {
protected:

    bool init(std::shared_ptr<ButtonSetting> setting, float width) {
        if (!SettingNodeV3::init(setting, width))
            return false;

        auto btn = Button::createWithSpriteFrameName("GJ_optionsBtn_001.png", [](Button*) {
            SettingsPopup::create()->show();
        });
        btn->setScale(0.475f);

        this->getButtonMenu()->addChildAtPosition(btn, Anchor::Center);
        this->getButtonMenu()->setContentWidth(60);
        this->getButtonMenu()->updateLayout();

        this->updateState(nullptr);

        return true;
    }

    void onCommit() override {}
    void onResetToDefault() override {}

public:

    static ButtonSettingNode* create(std::shared_ptr<ButtonSetting> setting, float width) {
        auto ret = new ButtonSettingNode();

        if (ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    bool hasUncommittedChanges() const override {
        return false;
    }
    bool hasNonDefaultValue() const override {
        return false;
    }

};

SettingNodeV3* ButtonSetting::createNode(float width) {
    return ButtonSettingNode::create(
        std::static_pointer_cast<ButtonSetting>(shared_from_this()),
        width
    );
}

}