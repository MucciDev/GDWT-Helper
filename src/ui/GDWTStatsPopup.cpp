#include "GDWTStatsPopup.hpp"
#include "../core/State.hpp"
#include "../core/Utils.hpp"

using namespace geode::prelude;

bool GDWTStatsPopup::init() {
    if (!Popup::init(290.f, 180.f)) return false;

    const std::string title = g_state.currentLevelName.length() > 15        
        ? "Level Stats"
        : fmt::format("{} Stats", g_state.currentLevelName);
    this->setTitle(title.c_str());

    loadGlobalStats();

    auto layout = ColumnLayout::create()->setGap(12.0f)->setAxisReverse(true);
    auto menu = CCMenu::create();
    menu->setLayout(layout);
    menu->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 10 });

    auto bestLabel = CCLabelBMFont::create(fmt::format("BEST PACE: {:.1f} PPM", g_state.globalBestPPM).c_str(), "goldFont.fnt");
    bestLabel->setScale(0.40f);
    menu->addChild(bestLabel);

    auto avgLabel = CCLabelBMFont::create(fmt::format("Average Pace: {:.1f} PPM", g_state.globalAvgPPM).c_str(), "chatFont.fnt");
    menu->addChild(avgLabel);

    auto timeLabel = CCLabelBMFont::create(fmt::format("Total Grind Time: {:.1f} Mins", g_state.globalTotalMins).c_str(), "chatFont.fnt");
    menu->addChild(timeLabel);

    menu->updateLayout();
    m_mainLayer->addChild(menu);

    return true;
}

GDWTStatsPopup* GDWTStatsPopup::create() {
    auto ret = new GDWTStatsPopup();
    if (ret && ret->init()) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
