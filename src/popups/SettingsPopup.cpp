#include "SettingsPopup.hpp"
#include "ALLManager.hpp"

#include "layers/ProScrollLayer.hpp"

#include <Geode/ui/Button.hpp>

SettingsPopup* SettingsPopup::create() {
    auto ret = new SettingsPopup();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool SettingsPopup::init() {
    // if (!ALLManager::get().isLoggedIn()) {
    //     return false;
    // }

    Popup::init(290, 283);

    this->setTitle("Settings");

    auto bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(60);
    bg->setPosition({m_size.width / 2.f, 131});
    bg->setContentSize({m_size.width - 31, 228});

    m_mainLayer->addChild(bg);

    auto scroll = ProScrollLayer::create(bg->getContentSize());
    scroll->setAnchorPoint({0.5f, 1});
    scroll->setPosition(bg->getPosition() + CCPoint{0, bg->getContentHeight() / 2.f});
    scroll->m_contentLayer->setLayout(
        SimpleAxisLayout::create(Axis::Column)
            ->setGap(7.f)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setMainAxisAlignment(MainAxisAlignment::Start)
    );

    m_mainLayer->addChild(scroll);

    scroll->m_contentLayer->addChild(AxisGap::create(7.f));

    const auto createContainer = [bg](ZStringView title, std::string_view description) -> CCNode* {
        auto container = CCMenu::create();
        container->setContentSize({bg->getContentWidth() - 14.f, 32.f});

        auto containerBg = NineSlice::create("square02b_001.png");
        containerBg->setScale(0.9f);
        containerBg->setContentSize(container->getContentSize() / containerBg->getScale());
        containerBg->setAnchorPoint({0, 0});
        containerBg->setColor({0, 0, 0});
        containerBg->setOpacity(30);

        container->addChild(containerBg);
       
        auto sub = CCNode::create();
        sub->setPosition({10.f, container->getContentHeight() / 2.f + 1});
        sub->setAnchorPoint({0, 0.5f});
        sub->setLayout(
            SimpleAxisLayout::create(Axis::Row)
                ->setGap(5.f)
                ->setMainAxisScaling(AxisScaling::Grow)
                ->setCrossAxisScaling(AxisScaling::Grow)
                ->setMainAxisAlignment(MainAxisAlignment::Start)
        );

        container->addChild(sub);
        
        auto lbl = CCLabelBMFont::create(title.c_str(), "bigFont.fnt");
        lbl->limitLabelWidth(container->getContentWidth() - 80.f, 0.39f, 0.f);

        sub->addChild(lbl);

        auto btn = Button::createWithSpriteFrameName("GJ_infoIcon_001.png", [title, description](Button*) {
            FLAlertLayer::create(
                title.c_str(),
                std::string(description),
                "Ok"
            )->show();
        });
        btn->setScale(0.5f);

        sub->addChild(btn);

        sub->updateLayout();
        
        return container;
    };

    const auto createToggleSetting = [scroll, createContainer](ZStringView title, std::string_view id, std::string_view description) {
        auto container = createContainer(title, description);

        auto toggle = CCMenuItemExt::createTogglerWithStandardSprites(0.525f, [id](CCMenuItemToggler* toggle) {
            Mod::get()->setSavedValue(id, !toggle->isToggled());
        });
        toggle->setPosition({container->getContentWidth() - 10.f - toggle->getScaledContentWidth() / 2.f, container->getContentHeight() / 2.f});
        toggle->toggle(Mod::get()->getSavedValue<bool>(id));

        container->addChild(toggle);

        scroll->m_contentLayer->addChild(container);
    };

    auto container = createContainer("API Base URL", "Base URL for the backend");

    auto input = TextInput::create(200, "...");
    input->setScale(0.5f);
    input->setPosition({container->getContentWidth() - 10.f - input->getScaledContentWidth() / 2.f, container->getContentHeight() / 2.f});
    input->setCommonFilter(CommonFilter::Any);
    input->setString(ALLManager::get().getBaseURL());

    container->addChild(input);

    scroll->m_contentLayer->addChild(container);

    createToggleSetting("Auto Sync Completions", "auto-sync-completions", "Automatically sync level completions in the background");
    createToggleSetting("Copy Art Button", "copy-art-button", "Show the copy art button in the editor pause, used to easily add decorations to your profile in the website");
    createToggleSetting("Report Level Info", "report-level-info", "Send level information of the levels you play to make the information in the website more accurrate. Only sends Two Player status for now.");

    scroll->m_contentLayer->updateLayout();
    scroll->scrollToTop();

    return true;
}