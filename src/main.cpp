#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>

using namespace geode::prelude;

struct HackDef { const char* label; const char* key; };
static constexpr HackDef HACKS[] = {
    {"Noclip",     "noclip"},
    {"Speedhack",  "speedhack"},
    {"Auto-Retry", "autoretry"},
};

static bool on(char const* key) { return Mod::get()->getSavedValue<bool>(key, false); }
static float speed() { return Mod::get()->getSavedValue<float>("speed", 1.f); }

class AdminPanel : public Popup<> {
protected:
    bool setup() override {
        this->setTitle("Admin Panel");
        auto size = m_mainLayer->getContentSize();

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        float y = size.height - 55.f;
        for (int i = 0; i < std::size(HACKS); i++) {
            auto toggler = CCMenuItemToggler::createWithStandardSprites(
                this, menu_selector(AdminPanel::onToggle), 0.6f);
            toggler->setTag(i);
            toggler->toggle(on(HACKS[i].key));
            toggler->setPosition({size.width - 40.f, y});
            menu->addChild(toggler);

            auto label = CCLabelBMFont::create(HACKS[i].label, "bigFont.fnt");
            label->setScale(0.5f);
            label->setAnchorPoint({0.f, 0.5f});
            label->setPosition({25.f, y});
            m_mainLayer->addChild(label);
            y -= 32.f;
        }

        m_speedLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_speedLabel->setScale(0.4f);
        m_speedLabel->setPosition({size.width / 2, y - 5.f});
        m_mainLayer->addChild(m_speedLabel);

        auto slider = Slider::create(this, menu_selector(AdminPanel::onSpeed), 0.7f);
        slider->setPosition({size.width / 2, y - 28.f});
        slider->setValue((speed() - 0.1f) / 2.9f);
        m_mainLayer->addChild(slider);
        updateLabel();
        return true;
    }

    void onToggle(CCObject* sender) {
        auto t = static_cast<CCMenuItemToggler*>(sender);
        Mod::get()->setSavedValue<bool>(HACKS[t->getTag()].key, !t->isToggled());
    }

    void onSpeed(CCObject* sender) {
        float v = static_cast<SliderThumb*>(sender)->getValue();
        Mod::get()->setSavedValue<float>("speed", 0.1f + v * 2.9f);
        updateLabel();
    }

    void updateLabel() {
        m_speedLabel->setString(fmt::format("Speed: {:.2f}x", speed()).c_str());
    }

    CCLabelBMFont* m_speedLabel = nullptr;

public:
    static AdminPanel* create() {
        auto ret = new AdminPanel();
        if (ret->initAnchored(280.f, 200.f)) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class $modify(PanelMenu, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Hacks"), this, menu_selector(PanelMenu::onPanel));
        auto menu = this->getChildByID("bottom-menu");
        menu->addChild(btn);
        menu->updateLayout();
        return true;
    }
    void onPanel(CCObject*) { AdminPanel::create()->show(); }
};

class $modify(PanelPause, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Hacks"), this, menu_selector(PanelPause::onPanel));
        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({60.f, 30.f});
        this->addChild(menu);
    }
    void onPanel(CCObject*) { AdminPanel::create()->show(); }
};

class $modify(HackPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (on("noclip") && obj != m_anticheatSpike) return;
        PlayLayer::destroyPlayer(player, obj);
    }
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (on("autoretry") && m_player1->m_isDead) this->resetLevel();
    }
};

class $modify(HackScheduler, CCScheduler) {
    void update(float dt) {
        CCScheduler::update(on("speedhack") ? dt * speed() : dt);
    }
};
