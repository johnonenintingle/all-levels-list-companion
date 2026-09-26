#include "CopyArtPopup.hpp"

#include "nodes/AreaTextInput.hpp"

#include <Geode/ui/Button.hpp>

CopyArtPopup* CopyArtPopup::create(LevelEditorLayer* editorLayer) {
    auto ret = new CopyArtPopup();

    if (ret->init(editorLayer)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool CopyArtPopup::init(LevelEditorLayer* editorLayer) {
    if (!editorLayer->m_editorUI) {
        return false;
    }

    auto selectedObject = editorLayer->m_editorUI->m_selectedObject;
    auto selectedObjects = editorLayer->m_editorUI->m_selectedObjects;
    
    if (!selectedObject && !selectedObjects) {
        return false;
    }

    auto mainObj = matjson::Value{};
    auto objects = matjson::Value::array();
    auto usedColorChannels = std::unordered_set<int>{};

    for (int i = 1; i < 20; i++) {
        usedColorChannels.insert(1000 + i);
    }
    
    const auto goThrough = [&](GameObject* object) {
        if (!object) {
            return;
        }

        auto json = matjson::Value{};

        json["id"] = object->m_objectID;
        json["xPos"] = object->getPositionX();
        json["yPos"] = object->getPositionY() - 90.f;
        json["xScale"] = object->getScaleX();
        json["yScale"] = object->getScaleY();
        json["rotation"] = object->getRotation();
        json["hidden"] = object->m_isHide;  

        auto groups = matjson::Value::array();

        if (object->m_groups) {
            for (auto group : *object->m_groups) {
                groups.push(group);
            }
        }

        json["groups"] = groups;

        if (object->m_baseColor) {
            usedColorChannels.insert(object->m_baseColor->m_colorID);
            json["baseColorChannel"] = object->m_baseColor->m_colorID;
        }

        if (object->m_detailColor) {
            usedColorChannels.insert(object->m_detailColor->m_colorID);
            json["detailColorChannel"] = object->m_detailColor->m_colorID;
        }

        objects.push(json);
    };

    if (selectedObjects) {
        for (auto object : CCArrayExt<GameObject*>(selectedObjects)) {
            goThrough(object);
        }
    }

    if (selectedObject) {
        goThrough(selectedObject);
    }

    if (objects.size() > 0) {
        mainObj["objects"] = objects;
    }

    auto colorChannels = matjson::Value{};

    for (auto id : usedColorChannels) {
        if (id == 1005 || id == 1006) {
            auto gm = GameManager::get();
            colorChannels[numToString(id)] = gm->colorForIdx(id == 1005 ? gm->getPlayerColor() : gm->getPlayerColor2());
            continue;
        }
        
        if (auto action = editorLayer->m_levelSettings->m_effectManager->getColorAction(id)) {
            if (id > 0) {
                colorChannels[numToString(id)] = action->m_fromColor;
            }
        }
    }

    mainObj["colorChannels"] = colorChannels;

    auto string = mainObj.dump(0);

    if (string.size() <= 2) {
        return false;
    }

    Popup::init(240, 240);

    this->setTitle("Copy Art");

    auto lbl = CCLabelBMFont::create("Paste this in your All Levels List", "bigFont.fnt");
    lbl->setPosition({m_size.width / 2.f, 199});
    lbl->setOpacity(177);
    lbl->setScale(0.3f);
    
    m_mainLayer->addChild(lbl);

    lbl = CCLabelBMFont::create("profile to decorate it", "bigFont.fnt");
    lbl->setPosition({m_size.width / 2.f, 188});
    lbl->setOpacity(177);
    lbl->setScale(0.3f);
    
    m_mainLayer->addChild(lbl);

    auto bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(55);
    bg->setPosition({m_size.width / 2.f, 108});
    bg->setContentSize({205, 139});

    m_mainLayer->addChild(bg);

    lbl = CCLabelBMFont::create("Objects String", "bigFont.fnt");
    lbl->setAnchorPoint({0, 0.5f});
    lbl->setPosition({26, 167});
    lbl->setScale(0.29f);
    lbl->setOpacity(180);

    m_mainLayer->addChild(lbl);

    auto copySpr = CCSprite::create("copy.png"_spr);
    copySpr->setOpacity(180);
    copySpr->setScale(0.4f);
    copySpr->setPosition({207, 166.76f});

    m_mainLayer->addChild(copySpr);

    auto checkmark = CCSprite::create("checkmark.png"_spr);
    checkmark->setOpacity(180);
    checkmark->setScale(0.4f);
    checkmark->setPosition({207, 166.3f});
    checkmark->setVisible(false);

    m_mainLayer->addChild(checkmark);

    lbl = CCLabelBMFont::create("Copy", "bigFont.fnt");
    lbl->setAnchorPoint({1, 0.5f});
    lbl->setPosition({197, 167});
    lbl->setScale(0.29f);
    lbl->setOpacity(180);

    m_mainLayer->addChild(lbl);

    auto line = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    line->setScaleX(0.43f);
    line->setScaleY(0.625f);
    line->setOpacity(80);
    line->setPosition({m_size.width / 2.f, 157});
    
    m_mainLayer->addChild(line);

    auto btn = Button::create([lbl, copySpr, checkmark, string](Button*) {
        if (!utils::clipboard::write(string)) {
            Notification::create("Couldn't write to clipboard", NotificationIcon::Error)->show();
            return;
        }
        
        lbl->setString("Copied");
        copySpr->setVisible(false);
        checkmark->setVisible(true);
    });
    btn->setPosition({190, 168});
    btn->setContentSize({94, 28});
    
    m_mainLayer->addChild(btn);

    btn = Button::createWithNode(ButtonSprite::create("Close"), [this](Button*) {
        this->onClose(nullptr);
    });
    btn->setScale(0.64f);
    btn->setScaleMultiplier(1.1f);
    btn->setPosition({m_size.width / 2.f, 22});

    m_mainLayer->addChild(btn);

    if (string.size() > 650) {
        string = string.substr(0, 650) + "...";
    }

    auto area = AreaTextInput::create({189, 105}, [](const std::string&) {});
    area->setOpacity(140);
    area->setScale(1.12f);
    area->setText(string);
    area->setEnabled(false);
    area->setPosition({m_size.width / 2.f, 99});
    area->scrollToTop();

    m_mainLayer->addChild(area);
    
    return true;

}