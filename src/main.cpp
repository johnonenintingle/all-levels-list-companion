#include "Includes.hpp"
#include "ALLManager.hpp"

#include "popups/LoginPopup.hpp"
#include "popups/ProfilePopup.hpp"
#include "popups/RankLevelPopup.hpp"
#include "popups/RateLevelPopup.hpp"
#include "popups/CopyArtPopup.hpp"

#include "nodes/ButtonSetting.hpp"
#include "nodes/LoaderButton.hpp"

#include "layers/ALLListLayer.hpp"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/AccountLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>

#include <Geode/ui/Button.hpp>
// #include <raydeeux.pages_api/include/PageMenu.h>

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
    
    for (auto level : getCompletedLevels()) {
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
        btn->setScale(0.63f);
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

        if (m_level->m_unlisted) {
            return;
        }

        auto id = m_level->m_levelID.value();

        if (!ALLManager::get().isLoggedIn() || id <= 0) {
            return true;
        }

        auto menu = this->getChildByID("left-side-menu");

        if (!menu) {
            return true;
        }

        auto rateButton = LoaderButton::create("rate.png"_spr, 0.95f, {0, 2.5f}, [this] {
            if (auto popup = RateLevelPopup::create(m_level)) {
                popup->show();
            }
        });
        rateButton->setID("rate-button"_spr);

        menu->addChild(rateButton, 2);

        auto rankButton = LoaderButton::create("compare.png"_spr, 1.f, {0.75f, 0.85f}, [this] {
            if (auto popup = RankLevelPopup::create(m_level)) {
                popup->show();
            }
        });
        rankButton->setID("compare-button"_spr);

        menu->addChild(rankButton, 1);

        menu->updateLayout();

        ALLManager::get().isLevelInList(id, [this, selfref = WeakRef(this), id, level = m_level, rateButton, rankButton](Result<bool> res) {
            if (!selfref.lock()) {
                return;
            }

            if (!res.isOk()) {
                rateButton->setError("Failed to get level in the list");
                rankButton->setError("Failed to get level in the list");
                return;
            }

            if (!res.unwrap()) {
                ALLManager::get().tryAddLevel(id, level, [this, selfref = WeakRef(this), rateButton, rankButton](Result<bool> res) {
                    if (!selfref.lock()) {
                        return;
                    }

                    if (!res.isOk() || !res.unwrap()) {
                        rankButton->setError(res.err().value_or("Level is not in the list"));
                        rateButton->setError(res.err().value_or("Level is not in the list"));
                        return;
                    }

                    rankButton->setLoading(false);
                    rateButton->setLoading(false);
                });
            } else {
                rankButton->setLoading(false);
                rateButton->setLoading(false);
            }
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
        
        for (auto level : getCompletedLevels()) {
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

// class $modify(LevelSearchLayer) {

//     bool init(int p0) {
//         if (!LevelSearchLayer::init(p0)) {
//             return false;
//         }

//         if (p0 > 0) {
//             return true;
//         }

//         auto menu = this->getChildByIDRecursive("quick-search-menu");

//         if (!menu) {
//             return true;
//         }

//         auto btnSpr = SearchButton::create("GJ_longBtn04_001.png", "ALL ", 0.506f, "accountBtn_blocked_001.png");
// 		btnSpr->getChildByIndex<CCNode*>(1)->setVisible(false);
		
// 		auto spr = CCSprite::create("all_logo_2.png"_spr);
//         spr->setScale(0.44f);
// 		spr->setPosition({82.1f, btnSpr->getContentHeight() / 2.f + 0.35f});

// 		btnSpr->addChild(spr);

// 		auto btn = Button::createWithNode(btnSpr, [this](Button*) {
//             m_searchInput->onClickTrackNode(false);
//             CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, ALLListLayer::scene()));
//         });

//         menu->addChild(btn);

//         if (Loader::get()->isModLoaded("alphalaneous.random_tab")) {
//             menu->updateLayout();
//             return true;
//         }

// 		menu->setContentSize({365, 116});
// 		menu->ignoreAnchorPointForPosition(false);
// 		menu->setPosition({ menu->getPositionX(), CCDirector::get()->getWinSize().height / 2.f + 28.f });
// 		menu->setLayout(
//             RowLayout::create()
//                 ->setGrowCrossAxis(true)
//                 ->setCrossAxisOverflow(false)
//                 ->setAxisAlignment(AxisAlignment::Center)
//                 ->setCrossAxisAlignment(AxisAlignment::Center)
//                 ->ignoreInvisibleChildren(true)
//         );

// 		static_cast<PageMenu*>(menu)->setPaged(9, PageOrientation::HORIZONTAL, 422);

//         return true;
//     }

// };