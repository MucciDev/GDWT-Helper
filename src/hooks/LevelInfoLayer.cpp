#include <Geode/modify/LevelInfoLayer.hpp>
#include "../ui/GDWTPopup.hpp"
#include "../core/State.hpp"
#include <cctype>

using namespace geode::prelude;

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto menu = this->getChildByID("left-side-menu");
        if (menu) {
            auto btn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("GDWT"),
                this, menu_selector(MyLevelInfoLayer::onGDWTPopup)
            );
            btn->setID("gdwt-button"_spr);
            menu->addChild(btn);
            menu->updateLayout();
        }
        return true;
    }

    void onGDWTPopup(CCObject*) {
        g_state.currentLevelName = this->m_level->m_levelName;
        const int id = this->m_level->m_levelID.value();

        // Strip invalid characters for file paths
        std::string cleanName = "";
        for (char c : g_state.currentLevelName) {
            if (cleanName.size() >= 64) break;

            const auto uc = static_cast<unsigned char>(c);
            if (std::isalnum(uc) || c == '-' || c == '_') cleanName += c;
            else if (c == ' ') cleanName += "_";
        }

        if (id > 0) g_state.currentLevelKey = fmt::format("{}_{}", id, cleanName);
        else g_state.currentLevelKey = cleanName.empty() ? "Local_Level" : cleanName;

        GDWTPopup::create()->show();
    }
};
