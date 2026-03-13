#include "GDWTPopup.hpp"
#include "GDWTStatsPopup.hpp"
#include "GDWTSessionViewer.hpp"
#include "../core/State.hpp"
#include "../core/Utils.hpp"

using namespace geode::prelude;

bool GDWTPopup::init() {
    if (!Popup::init(260.0f, 290.0f)) return false;
    this->setTitle("Configure GDWT");

    auto inputMenu = CCMenu::create();
    inputMenu->setLayout(ColumnLayout::create()->setGap(4.0f)->setAxisReverse(true));
    inputMenu->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 + 25.0f });

    m_timeInput = TextInput::create(180, "Time (Minutes)");
    m_timeInput->setFilter("0123456789.");
    inputMenu->addChild(m_timeInput);

    m_qualInput = TextInput::create(180, "Qualifying % (e.g. 25)");
    m_qualInput->setFilter("0123456789.");
    inputMenu->addChild(m_qualInput);

    m_15Input = TextInput::create(180, "x1.5 % (e.g. 50)");
    m_15Input->setFilter("0123456789.");
    inputMenu->addChild(m_15Input);

    m_20Input = TextInput::create(180, "x2.0 % (e.g. 75)");
    m_20Input->setFilter("0123456789.");
    inputMenu->addChild(m_20Input);

    inputMenu->updateLayout();
    m_mainLayer->addChild(inputMenu);

    auto topBtnMenu = CCMenu::create();
    topBtnMenu->setLayout(RowLayout::create()->setGap(8.0f));
    topBtnMenu->setPosition({ m_mainLayer->getContentSize().width / 2, 65.0f });

    auto viewBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Sessions", "goldFont.fnt", "GJ_button_02.png", 0.65f),
        this, menu_selector(GDWTPopup::onView)
    );
    topBtnMenu->addChild(viewBtn);

    auto statsBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Pace", "goldFont.fnt", "GJ_button_02.png", 0.65f),
        this, menu_selector(GDWTPopup::onStats)
    );
    topBtnMenu->addChild(statsBtn);

    topBtnMenu->updateLayout();
    m_mainLayer->addChild(topBtnMenu);

    auto bottomBtnMenu = CCMenu::create();
    bottomBtnMenu->setPosition({ m_mainLayer->getContentSize().width / 2, 30.0f });

    auto startBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Ready", "goldFont.fnt", "GJ_button_01.png", 0.65f),
        this, menu_selector(GDWTPopup::onStart)
    );
    bottomBtnMenu->addChild(startBtn);

    m_mainLayer->addChild(bottomBtnMenu);

    return true;
}

void GDWTPopup::onStart(CCObject*) {
    // Reset state and apply inputs
    g_state.sessionDurationMinutes = utils::numFromString<float>(m_timeInput->getString()).unwrapOr(DEFAULT_TIMER_MINUTES);
    g_state.qualifyPct = utils::numFromString<float>(m_qualInput->getString()).unwrapOr(25.0f);
    g_state.p15Pct = utils::numFromString<float>(m_15Input->getString()).unwrapOr(50.0f);
    g_state.p20Pct = utils::numFromString<float>(m_20Input->getString()).unwrapOr(75.0f);

    g_state.timeRemaining = g_state.sessionDurationMinutes * SECONDS_PER_MINUTE;
    g_state.timeElapsed = 0.0f;
    g_state.totalPoints = 0.0f;
    g_state.currentPPM = 0.0f;
    g_state.allPercentages.clear();
    g_state.currentStreakCompletions = 0;
    g_state.bestStreakPct = 0;

    g_state.isPendingStart = true;
    g_state.isActive = false;
    g_state.timeExpired = false;

    sendServerPing("start");
    loadGlobalStats();

    FLAlertLayer::create("Simulation Ready", "Simulation configured! Hit Play to start.", "OK")->show();
    this->onClose(nullptr);
}

void GDWTPopup::onStats(CCObject*) { GDWTStatsPopup::create()->show(); }
void GDWTPopup::onView(CCObject*) { GDWTSessionViewer::create()->show(); }

GDWTPopup* GDWTPopup::create() {
    auto ret = new GDWTPopup();
    if (ret && ret->init()) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
