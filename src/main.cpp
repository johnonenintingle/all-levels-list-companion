#include "Includes.hpp"
#include "ALLManager.hpp"

#include "popups/LoginPopup.hpp"
#include "popups/ProfilePopup.hpp"
#include "popups/RankLevelPopup.hpp"
#include "popups/RateLevelPopup.hpp"
#include "popups/CopyArtPopup.hpp"

#include "nodes/ButtonSetting.hpp"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/AccountLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

#include <Geode/ui/Button.hpp>

$on_mod(Loaded) {
    (void)Mod::get()->registerCustomSettingType("button-setting", &ButtonSetting::parse);
        
    auto& all = ALLManager::get();

    if (!Mod::get()->setSavedValue("did-set-defaults3", true)) {
        Mod::get()->setSavedValue("auto-sync-completions", true);
        Mod::get()->setSavedValue("copy-art-button", true);
        Mod::get()->setSavedValue("report-level-info", true);
        Mod::get()->setSavedValue<std::string>("api-base-url", "https://alllevelslist.net");
    }

    if (Mod::get()->hasSavedValue("account-token")) {
        all.setToken(Mod::get()->getSavedValue<std::string>("account-token", ""));
    
        all.setUser(UserInfo{
            .id = Mod::get()->getSavedValue<int>("account-id", 0),
            .username = Mod::get()->getSavedValue<std::string>("account-username", ""),
            .avatarUrl = Mod::get()->getSavedValue<std::string>("account-avatar-url", "")
        });
    }

    if (all.isLoggedIn()) {
        all.updateUserInfo();

        auto preloadSprite = LazySprite::create({0, 0}, false);
        preloadSprite->loadFromUrl(all.getUser().avatarUrl, LazySprite::Format::kFmtWebp, false);
        preloadSprite->setLoadCallback([_ = Ref(preloadSprite)](Result<>) {});
    
        all.setLinkedGDAccount(
            Mod::get()->getSavedValue<int>("linked-gd-account-id", 0),
            Mod::get()->getSavedValue<std::string>("linked-gd-account-username", "")
        );
    }

    for (const auto& v : Mod::get()->getSavedValue<matjson::Value>("pending-completions")) {
        auto id = v.asInt().unwrapOr(0);

        if (id > 0) {
            all.m_pendingCompletions.insert(id);
        }
    }

    for (const auto& v : Mod::get()->getSavedValue<matjson::Value>("registered-completions")) {
        auto id = v.asInt().unwrapOr(0);

        if (id > 0) {
            all.m_registeredCompletions.insert(id);
        }
    }
    
    for (auto level : CCArrayExt<GJGameLevel*>(GameLevelManager::get()->getCompletedLevels(false))) {
        auto id = level->m_levelID.value();

        if (id > 0 && !all.m_registeredCompletions.contains(id)) {
            all.m_pendingCompletions.insert(id);
        }
    }

    all.savePendingCompletions();

    if (Mod::get()->getSavedValue<bool>("auto-sync-completions")) {
        all.syncPendingCompletions(true);
    }
    
    if (all.isLoggedIn() && !all.isGDAccountLinked()) {
        all.linkGDAccount();
    }

    for (const auto& v : Mod::get()->getSavedValue<matjson::Value>("levels-data-sent")) {
        auto id = v.asInt().unwrapOr(0);

        if (id > 0) {
            all.m_levelsDataSent.insert(id);
        }
    }

    for (const auto& v : Mod::get()->getSavedValue<matjson::Value>("levels-in-list")) {
        auto id = v.asInt().unwrapOr(0);

        if (id > 0) {
            all.setLevelInList(id, true);
        }
    }
}

class $modify(MenuLayer) {

    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto btn = Button::createWithSprite("all_logo.png"_spr, [](Button*) {
            if (ALLManager::get().isLoggedIn()) {
                if (auto popup = ProfilePopup::create()) {
                    popup->show();
                }
            } else {
                LoginPopup::create()->show();

                if (ALLManager::get().m_sessionExpired) {
                    ALLManager::get().m_sessionExpired = false;
                    Notification::create("Invalid session, please log in again", NotificationIcon::Error)->show();
                }
            }
        });
        btn->setID("all-button"_spr);
        btn->setScale(0.7f);
        btn->setScaleMultiplier(1.11f);
        btn->setPosition(this->getContentSize() - btn->getScaledContentSize() / 2.f - CCPoint{9, 9});

        this->addChild(btn);

        return true;
    }

};

class $modify(LevelInfoLayer) {

    bool init(GJGameLevel* p0, bool p1) {
        if (!LevelInfoLayer::init(p0, p1)) {
            return false;
        }

        auto id = p0->m_levelID.value();

        if (!ALLManager::get().isLoggedIn() || id <= 0) {
            return true;
        }

        ALLManager::get().isLevelInList(id, [this, selfref = WeakRef(this)](Result<bool> res) {
            if (!selfref.lock()) {
                return;
            }

            if (!res.isOk() || !res.unwrap()) {
                return;
            }

            auto menu = this->getChildByID("left-side-menu");

            if (!menu) {
                return;
            }

            auto plain = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");

            auto spr = CCSprite::create("rate.png"_spr);
            spr->setPosition(plain->getContentSize() / 2.f + CCPoint{0, 2.5f});
            spr->setScale(0.95f);

            plain->addChild(spr);

            auto btn = Button::createWithNode(plain, [this](Button*) {
                if (auto popup = RateLevelPopup::create(m_level)) {
                    popup->show();
                }
            });
            btn->setID("rate-button"_spr);

            menu->addChild(btn, 2);

            plain = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");

            spr = CCSprite::create("compare.png"_spr);
            spr->setPosition(plain->getContentSize() / 2.f + CCPoint{0.75f, 0.85f});

            plain->addChild(spr);

            btn = Button::createWithNode(plain, [this](Button*) {
                if (auto popup = RankLevelPopup::create(m_level)) {
                    popup->show();
                }
            });
            btn->setID("compare-button"_spr);

            menu->addChild(btn, 1);
            menu->updateLayout();
        });

        return true;
    }

    void confirmDelete(CCObject* p0) {
        ALLManager::get().uncacheLevelRating(m_level->m_levelID.value());
        ALLManager::get().removeUserListsCache();
        LevelInfoLayer::confirmDelete(p0);
    }

    void onUpdate(CCObject* p0) {
        LevelInfoLayer::onUpdate(p0);
        ALLManager::get().uncacheLevelRating(m_level->m_levelID.value());
        ALLManager::get().removeUserListsCache();
    }

};

class $modify(PlayLayer) {

    void levelComplete() {
        PlayLayer::levelComplete();

        auto& all = ALLManager::get();
        auto id = m_level->m_levelID.value();

        if (
            id <= 0
            || all.m_pendingCompletions.contains(id)
            || all.m_registeredCompletions.contains(id)
            || !GameStatsManager::get()->hasCompletedLevel(m_level)
        ) {
            return;
        }

        all.addCompletion(id);
    }

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        if (Mod::get()->getSavedValue<bool>("report-level-info")) {
            ALLManager::get().trySendLevelData(m_level);
        }
    }

};

class $modify(AccountLayer) {

    void syncAccountFinished() {
        AccountLayer::syncAccountFinished();

        auto& all = ALLManager::get();
        
        for (auto level : CCArrayExt<GJGameLevel*>(GameLevelManager::get()->getCompletedLevels(false))) {
            auto id = level->m_levelID.value();

            if (id > 0 && !all.m_registeredCompletions.contains(id)) {
                all.m_pendingCompletions.insert(id);
            }
        }

        all.savePendingCompletions();

        if (Mod::get()->getSavedValue<bool>("auto-sync-completions")) {
            all.syncPendingCompletions(true);
        }
    }

};

class $modify(EditorPauseLayer) {

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorPauseLayer::init(editorLayer)) {
            return false;
        }

        if (!editorLayer) {
            return true;
        }

        if (
            !editorLayer->m_editorUI->m_selectedObject
            && (
                !editorLayer->m_editorUI->m_selectedObjects
                || editorLayer->m_editorUI->m_selectedObjects->count() <= 0
            )
        ) {
            return true;
        }

        auto menu = this->getChildByID("guidelines-menu");

        if (!menu) {
            return true;
        }

        auto plain = CircleButtonSprite::create(nullptr, CircleBaseColor::Green, CircleBaseSize::Medium);
        plain->setScale(0.82f);

        auto spr = CCSprite::create("brush.png"_spr);
        spr->setPosition(plain->getContentSize() / 2.f);

        plain->addChild(spr);

        auto btn = Button::createWithNode(plain, [this](Button*) {
            auto string = std::string{};

            if (m_editorLayer->m_editorUI->m_selectedObjects) {
                for (auto object : CCArrayExt<GameObject*>(m_editorLayer->m_editorUI->m_selectedObjects)) {
                    string += std::string(object->getSaveString(m_editorLayer)) + ";";
                }
            }

            if (m_editorLayer->m_editorUI->m_selectedObject) {
                string += std::string(m_editorLayer->m_editorUI->m_selectedObject->getSaveString(m_editorLayer)) + ";";
            }

            if (!string.empty()) {
                CopyArtPopup::create(std::move(string))->show();
            }
        });
        btn->setID("copy-art-button"_spr);
        btn->setScaleMultiplier(1.1f);

        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

};