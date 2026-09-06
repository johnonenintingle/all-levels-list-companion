#include "ProfilePopup.hpp"
#include "ALLManager.hpp"

#include "popups/LoginPopup.hpp"
#include "popups/SettingsPopup.hpp"
#include "nodes/AvatarNode.hpp"

ProfilePopup* ProfilePopup::create() {
    auto ret = new ProfilePopup();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool ProfilePopup::init() {
    const auto& user = ALLManager::get().getUser();

    if (user.id <= 0) {
        return false;
    }
    
    Popup::init(343, 228);

    this->setTitle("All Levels List", "goldFont.fnt", 0.7f, 21.f);

    auto bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(65);
    bg->setContentSize({309, 59});
    bg->setPosition({m_size.width / 2.f, 159});

    m_mainLayer->addChild(bg);

    bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(65);
    bg->setContentSize({309, 105});
    bg->setPosition({m_size.width / 2.f, 67});

    m_mainLayer->addChild(bg);

    auto lbl = CCLabelBMFont::create("Account", "bigFont.fnt");
    lbl->setPosition({m_size.width / 2.f, 179});
    lbl->setScale(0.375f);
    lbl->setOpacity(186);

    m_mainLayer->addChild(lbl);

    auto line = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    line->setScaleX(0.65f);
    line->setScaleY(0.7f);
    line->setOpacity(90);
    line->setPosition({m_size.width / 2.f, 167});

    m_mainLayer->addChild(line);
    
    auto pfp = AvatarNode::create(user.avatarUrl);
    pfp->setPosition({40, 148});

    m_mainLayer->addChild(pfp);

    lbl = CCLabelBMFont::create("Logged in as ", "bigFont.fnt");
    lbl->setAnchorPoint({0, 0.5f});
    lbl->setScale(0.325f);
    lbl->setOpacity(205);
    lbl->setPosition({60, 149});

    m_mainLayer->addChild(lbl);

    auto usernameLbl = CCLabelBMFont::create(user.username.c_str(), "bigFont.fnt");
    usernameLbl->setAnchorPoint({0, 0.5f});
    usernameLbl->limitLabelWidth(104.568f, 0.325f, 0.f);
    usernameLbl->setColor({134, 233, 110});
    usernameLbl->setPosition({lbl->getPositionX() + lbl->getScaledContentWidth(), 149.f - (0.325f - usernameLbl->getScale()) / 0.325f});

    m_mainLayer->addChild(usernameLbl);
    
    auto btn = Button::createWithNode(ButtonSprite::create("Log out", "goldFont.fnt", "GJ_button_06.png"), [this](Button*) {
        createQuickPopup(
            "Warning",
            "Are you sure you want to <cr>Log Out</c>?",
            "Cancel", "Ok",
            [this](auto, bool btn2) {
                ALLManager::get().logout();
                this->onClose(nullptr);

                Notification::create("Logged out", NotificationIcon::Success)->show();
                
                queueInMainThread([] {
                    LoginPopup::create()->show();
                });
            }
        );
    });
    btn->setScaleMultiplier(1.13f);
    btn->setScale(0.57f);
    btn->setPosition({281, 148});

    m_mainLayer->addChild(btn);

    lbl = CCLabelBMFont::create("Completions", "bigFont.fnt");
    lbl->setPosition({m_size.width / 2.f, 110});
    lbl->setScale(0.375f);
    lbl->setOpacity(186);

    m_mainLayer->addChild(lbl);

    line = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    line->setScaleX(0.65f);
    line->setScaleY(0.7f);
    line->setOpacity(90);
    line->setPosition({m_size.width / 2.f, 98});

    m_mainLayer->addChild(line);

    m_pendingLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_pendingLabel->setAnchorPoint({0, 0.5f});
    m_pendingLabel->setPosition({28, 85});
    m_pendingLabel->setScale(0.525f);

    m_mainLayer->addChild(m_pendingLabel);

    auto btnSpr = ButtonSprite::create("Sync pending");
    btnSpr->setCascadeOpacityEnabled(true);

    m_syncPendingButton = Button::createWithNode(btnSpr, [this](Button*) {
        auto& all = ALLManager::get();
        
        all.syncPendingCompletions(false);

        if (all.isSyncingCompletions()) {
            this->setLoading(true);
            
            all.listenForSync([selfref = WeakRef(this)] {
                auto self = selfref.lock();

                if (!self) {
                    return;
                }

                self->setLoading(false);
                self->updatePendingCompletions();
            });
        }
    });
    m_syncPendingButton->setScaleMultiplier(1.13f);
    m_syncPendingButton->setScale(0.58f);
    m_syncPendingButton->setPosition({m_pendingLabel->getPositionX() + m_syncPendingButton->getScaledContentWidth() / 2.f, 59});

    m_mainLayer->addChild(m_syncPendingButton);

    this->updatePendingCompletions();

    btn = Button::createWithNode(ButtonSprite::create("Re-Sync all"), [this](Button*) {
        auto& all = ALLManager::get();
        
        all.syncAllCompletions();

        if (all.isSyncingCompletions()) {
            this->setLoading(true);
            
            all.listenForSync([selfref = WeakRef(this)] {
                auto self = selfref.lock();

                if (!self) {
                    return;
                }

                self->setLoading(false);
                self->updatePendingCompletions();
            });
        }
    });
    btn->setScaleMultiplier(1.13f);
    btn->setScale(0.58f);
    btn->setPosition({m_pendingLabel->getPositionX() + btn->getScaledContentWidth() / 2.f, 36});

    m_mainLayer->addChild(btn);

    btn = Button::createWithSpriteFrameName("GJ_optionsBtn_001.png", [](Button*) {
        SettingsPopup::create()->show();
    });
    btn->setScaleMultiplier(1.13f);
    btn->setScale(0.53f);
    btn->setPosition({302.5f, 38});

    m_mainLayer->addChild(btn);

    auto& all = ALLManager::get();

    if (all.isSyncingCompletions()) {
        this->setLoading(true);
        
        all.listenForSync([selfref = WeakRef(this)] {
            auto self = selfref.lock();

            if (!self) {
                return;
            }

            self->setLoading(false);
            self->updatePendingCompletions();
        });
    }

    return true;
}

void ProfilePopup::updatePendingCompletions() {
    auto count = ALLManager::get().m_pendingCompletions.size();
    ZStringView text;

    if (count == 0) {
    text = "No completions pending";
    } else if (count == 1) {
    text = "1 completion pending";
    } else {
    text = fmt::format("{} completions pending", count);
    }

    m_pendingLabel->setString(text.c_str());

    m_syncPendingButton->setEnabled(count > 0);
    m_syncPendingButton->setOpacity(count > 0 ? 255 : 120);
}