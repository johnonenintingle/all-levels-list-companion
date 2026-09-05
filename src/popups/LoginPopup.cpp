#include "LoginPopup.hpp"
#include "ALLManager.hpp"

#include "popups/ProfilePopup.hpp"
#include "nodes/UnderlinedLabel.hpp"
#include "layers/LoadingScreenLayer.hpp"

#include <Geode/ui/Button.hpp>

LoginPopup* LoginPopup::create() {
    auto ret = new LoginPopup();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool LoginPopup::init() {
    Popup::init(359, 230);

    this->setTitle("All Levels List", "goldFont.fnt", 0.7f, 21.f);

    auto lbl = CCLabelBMFont::create("Link your Geometry Dash account to All Levels List", "bigFont.fnt");
    lbl->setOpacity(180);
    lbl->setPosition({m_size.width / 2.f, 185});
    lbl->setScale(0.3f);

    m_mainLayer->addChild(lbl);
    
    auto bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(40);
    bg->setContentSize({330, 157});
    bg->setPosition({m_size.width / 2.f, 95});

    m_mainLayer->addChild(bg);

    auto container = CCNode::create();
    container->setAnchorPoint({0.5f, 1.f});
    container->setPosition({97, 169});
    container->setLayout(
        SimpleAxisLayout::create(Axis::Column)
            ->setGap(2.f)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setMainAxisAlignment(MainAxisAlignment::Start)
    );

    m_mainLayer->addChild(container);

    container->addChild(AxisGap::create(1.f));

    lbl = CCLabelBMFont::create("Log In", "goldFont.fnt");
    lbl->setScale(0.485f);
    lbl->setOpacity(230);

    container->addChild(lbl);
    container->addChild(AxisGap::create(17.f));

    lbl = CCLabelBMFont::create("Username", "bigFont.fnt");
    lbl->setScale(0.25f);
    lbl->setOpacity(192);

    container->addChild(lbl);

    auto usernameInput = TextInput::create(230, ". . .");
    usernameInput->setScale(0.585f);
    usernameInput->getBGSprite()->setOpacity(60);
    usernameInput->getInputNode()->getTextLabel()->setOpacity(120);
    usernameInput->setCallback([input = usernameInput](const std::string& str) {
        input->getInputNode()->getTextLabel()->setOpacity(str.empty() ? 120 : 255);
    });

    container->addChild(usernameInput);
    container->addChild(AxisGap::create(15.f));

    lbl = CCLabelBMFont::create("Password", "bigFont.fnt");
    lbl->setScale(0.25f);
    lbl->setOpacity(192);

    container->addChild(lbl);

    auto passwordInput = TextInput::create(230, ". . .");
    passwordInput->setPasswordMode(true);
    passwordInput->setScale(0.585f);
    passwordInput->getBGSprite()->setOpacity(60);
    passwordInput->getInputNode()->getTextLabel()->setOpacity(120);

    container->addChild(passwordInput);
    container->addChild(AxisGap::create(25.f));

    auto btnSpr = ButtonSprite::create("Log in");
    btnSpr->setCascadeOpacityEnabled(true);

    auto btn = Button::createWithNode(btnSpr, [this, usernameInput, passwordInput](Button*) {
        if (m_errorLbl) {
            m_errorLbl->setVisible(false);
        }

        auto username = usernameInput->getString();
        auto password = passwordInput->getString();

        this->setLoading(true);

        ALLManager::get().login(
            username, password,
            [selfref = WeakRef(this)](Result<> res) {
                if (auto self = selfref.lock()) {
                    self->onLogin(res);
                }
            }
        );
    });
    btn->setScale(0.63f);
    btn->setScaleMultiplier(1.13f);
    btn->setEnabled(false);
    btn->setOpacity(120);

    container->addChild(btn);

    usernameInput->setCallback([usernameInput, passwordInput, btn](const std::string& str) {
        usernameInput->getInputNode()->getTextLabel()->setOpacity(str.empty() ? 120 : 255);

        auto enabled = !str.empty() && !passwordInput->getString().empty();

        btn->setEnabled(enabled);
        btn->setOpacity(enabled ? 255 : 120);
    });

    passwordInput->setCallback([passwordInput, usernameInput, btn](const std::string& str) {
        passwordInput->getInputNode()->getTextLabel()->setOpacity(str.empty() ? 120 : 255);
        
        auto enabled = !str.empty() && !usernameInput->getString().empty();

        btn->setEnabled(enabled);
        btn->setOpacity(enabled ? 255 : 120);
    });

    container->updateLayout();

    auto line = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    line->setScaleX(0.35f);
    line->setScaleY(0.65f);
    line->setOpacity(64);
    line->setRotation(90);
    line->setPosition({190, bg->getPositionY()});

    m_mainLayer->addChild(line);

    auto layer = CCLayerColor::create({129, 72, 43, 255}, 20, 14);
    layer->ignoreAnchorPointForPosition(false);
    layer->setPosition({190, bg->getPositionY()});

    m_mainLayer->addChild(layer);

    lbl = CCLabelBMFont::create("or", "bigFont.fnt");
    lbl->setScale(0.3f);
    lbl->setOpacity(103);
    lbl->setPosition({190, bg->getPositionY() + 1.2f});

    m_mainLayer->addChild(lbl);

    lbl = CCLabelBMFont::create("Register", "goldFont.fnt");
    lbl->setScale(0.485f);
    lbl->setOpacity(230);
    lbl->setPosition({268.7f, 161.2f});

    m_mainLayer->addChild(lbl);

    m_websiteBg = NineSlice::create("square02b_001.png");
    m_websiteBg->setColor({0, 0, 0});
    m_websiteBg->setOpacity(34);
    m_websiteBg->setContentSize({101, 109});

    auto logo = CCSprite::create("all_logo.png"_spr);
    logo->setPosition(m_websiteBg->getContentSize() / 2.f + CCPoint{0, 17});
    logo->setScale(0.9f);
    logo->runAction(CCRepeatForever::create(CCSequence::create(
        CCEaseSineInOut::create(CCScaleTo::create(1.5f, 0.935f)),
        CCDelayTime::create(0.18f),
        CCEaseSineInOut::create(CCScaleTo::create(1.5f, 0.9f)),
        nullptr
    )));

    m_websiteBg->addChild(logo);

    auto underlinedLbl = UnderlinedLabel::create("Create an account", "bigFont.fnt");
    underlinedLbl->setScale(0.25f);
    underlinedLbl->setOpacity(126);
    underlinedLbl->setPosition(m_websiteBg->getContentSize() / 2.f + CCPoint{0, -23});

    m_websiteBg->addChild(underlinedLbl);

    underlinedLbl = UnderlinedLabel::create("in the website", "bigFont.fnt");
    underlinedLbl->setScale(0.25f);
    underlinedLbl->setOpacity(126);
    underlinedLbl->setPosition(m_websiteBg->getContentSize() / 2.f + CCPoint{0, -31});

    m_websiteBg->addChild(underlinedLbl);

    btn = Button::createWithNode(m_websiteBg, [](Button*) {
        web::openLinkInBrowser("https://alllevelslist.net/");
    });
    btn->setScaleMultiplier(1.18f);
    btn->setPosition({268.5f, 88});

    m_mainLayer->addChild(btn);

    #ifndef GEODE_IS_MOBILE
    
    btn->setScaleMultiplier(1.f);

    this->schedule(schedule_selector(LoginPopup::update), 1.f / 30.f, kCCRepeatForever, 0.1f);

    #endif

    auto& all = ALLManager::get();

    if (all.isLoggingIn()) {
        this->setLoading(true);

        all.listenForLogin([self = Ref(this)](Result<> res) {
            self->onLogin(res);
        });
    }

    return true;
}

void LoginPopup::onLogin(Result<> res) {
    if (res.isOk()) {
        this->onClose(nullptr);
        
        // queueInMainThread([] {
        //     if (auto popup = ProfilePopup::create()) {
        //         popup->show();
        //     }
        // });

        return;
    }

    this->setLoading(false);
    this->setError(res.err().value_or("An unknown error has occurred"));
}

void LoginPopup::setError(ZStringView error) {
    if (m_errorLbl) {
        m_errorLbl->setString(error.c_str());
        m_errorLbl->setVisible(true);
        return;
    }

    m_errorLbl = CCLabelBMFont::create(error.c_str(), "bigFont.fnt");
    m_errorLbl->setColor({200, 40, 40});
    m_errorLbl->limitLabelWidth(110.f, 0.23f, 0.f);
    m_errorLbl->setPosition({97, 56});

    m_mainLayer->addChild(m_errorLbl);
}

void LoginPopup::update(float) {
    auto hovering = isHoveringNode(getMousePos(), m_websiteBg) && !m_isLoading;

    if (hovering != m_hoveringWebsite) {
        m_hoveringWebsite = hovering;

        auto color = hovering ? ccColor3B{32, 32, 32} : ccColor3B{0, 0, 0};

        m_websiteBg->stopAllActions();
        m_websiteBg->runAction(CCSpawn::create(
            CCEaseSineInOut::create(CCTintTo::create(0.23f, color.r, color.g, color.b)),   
            CCEaseSineInOut::create(CCScaleTo::create(0.23f, hovering ? 1.035f : 1.f)),
            nullptr   
        ));
    }
}