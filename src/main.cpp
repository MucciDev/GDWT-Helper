#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>

using namespace geode::prelude;

// --- Global State ---
struct GDWTState {
    bool isPendingStart = false; 
    bool isActive = false;
    bool timeExpired = false; 
    
    // Level Identification
    std::string currentLevelKey = "Unknown";
    std::string currentLevelName = "Level";

    float timeRemaining = 0.0f; 
    float timeElapsed = 0.0f; 
    float sessionDurationMinutes = 0.0f;
    
    float qualifyPct = 32.0f;
    float p15Pct = 54.0f;
    float p20Pct = 75.0f;
    
    float totalPoints = 0.0f; 
    float currentPPM = 0.0f;
    std::vector<int> allPercentages;
    int currentStreakCompletions = 0;
    int bestStreakPct = 0;

    float globalBestPPM = 0.0f;
    float globalAvgPPM = 0.0f;
    float globalTotalMins = 0.0f;
};

inline GDWTState g_state;

// --- Helpers ---
std::string getCurrentDateString() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

void loadGlobalStats() {
    auto configDir = geode::Mod::get()->getConfigDir() / g_state.currentLevelKey;
    auto statsPath = configDir / "stats.txt";
    float bestPPM = 0.0f, totalPts = 0.0f, totalMins = 0.0f;

    std::ifstream inFile(statsPath);
    if (inFile.is_open()) {
        inFile >> bestPPM >> totalPts >> totalMins;
        inFile.close();
    }

    g_state.globalBestPPM = bestPPM;
    g_state.globalTotalMins = totalMins;
    g_state.globalAvgPPM = (totalMins > 0.0f) ? (totalPts / totalMins) : 0.0f;
}

float calculatePoints(float pct) {
    if (pct >= 100.0f) return 250.0f; 
    if (pct >= g_state.p20Pct) return pct * 2.0f;
    if (pct >= g_state.p15Pct) return pct * 1.5f;
    if (pct >= g_state.qualifyPct) return pct;
    return 0.0f;
}

void processSessionEnd() {
    g_state.isActive = false; 

    float elapsedMins = g_state.timeElapsed / 60.0f;
    float finalPPM = (elapsedMins > 0.0f) ? (g_state.totalPoints / elapsedMins) : 0.0f;

    std::string pctString = "";
    for (size_t i = 0; i < g_state.allPercentages.size(); ++i) {
        pctString += std::to_string(g_state.allPercentages[i]) + "%";
        if (i < g_state.allPercentages.size() - 1) {
            pctString += ", ";
            if ((i + 1) % 10 == 0) pctString += "\n"; 
        }
    }
    if (pctString.empty()) pctString = "No valid runs completed.";

    auto configDir = geode::Mod::get()->getConfigDir() / g_state.currentLevelKey;
    std::filesystem::create_directories(configDir);
    
    std::string dateStr = getCurrentDateString();

    auto runPath = configDir / fmt::format("Simulation_{}.txt", dateStr);
    std::ofstream runFile(runPath);
    if (runFile.is_open()) {
        runFile << "GDWT Simulation - " << dateStr << "\n";
        runFile << "Level: " << g_state.currentLevelName << "\n";
        runFile << "--------------------------------\n";
        runFile << "Total Points: " << std::fixed << std::setprecision(1) << g_state.totalPoints << "\n";
        runFile << "Pace: " << std::fixed << std::setprecision(1) << finalPPM << " PPM\n";
        runFile << "Best Streak: " << g_state.bestStreakPct << "%\n";
        runFile << "Duration: " << std::fixed << std::setprecision(2) << elapsedMins << " Minutes\n";
        runFile << "--------------------------------\n";
        runFile << "Run History:\n" << pctString << "\n";
        runFile.close();
    }

    float bestPPM = 0.0f, totalHistoricalPts = 0.0f, totalHistoricalMins = 0.0f;
    auto statsPath = configDir / "stats.txt";
    std::ifstream inFile(statsPath);
    if (inFile.is_open()) {
        inFile >> bestPPM >> totalHistoricalPts >> totalHistoricalMins;
        inFile.close();
    }
    if (finalPPM > bestPPM) bestPPM = finalPPM;
    totalHistoricalPts += g_state.totalPoints;
    totalHistoricalMins += elapsedMins;

    std::ofstream outFile(statsPath);
    if (outFile.is_open()) {
        outFile << bestPPM << " " << totalHistoricalPts << " " << totalHistoricalMins;
        outFile.close();
    }

    std::string uiPctString = pctString;
    if (uiPctString.length() > 80) {
        uiPctString = uiPctString.substr(0, 77) + "...";
    }

    auto msg = fmt::format(
        "Total Points: {:.1f}\nPace: {:.1f} PPM\nBest Streak: {}%\n\nLog saved to config folder!\n\nHistory:\n{}", 
        g_state.totalPoints, 
        finalPPM,
        g_state.bestStreakPct, 
        uiPctString
    );

    FLAlertLayer::create("Simulation Finished", msg, "OK")->show();
}

// --- UI: Global Stats ---
class GDWTStatsPopup : public geode::Popup {
protected:
    bool init() {
        if (!Popup::init(290.f, 180.f)) return false;
        
        std::string title = g_state.currentLevelName.length() > 15 
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
public:
    static GDWTStatsPopup* create() {
        auto ret = new GDWTStatsPopup();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// --- UI: Session History Viewer ---
class GDWTSessionViewer : public geode::Popup {
protected:
    // FIX: Using filesystem::path directly instead of strings
    std::vector<std::filesystem::path> m_files;
    int m_currentIndex = 0;
    geode::ScrollLayer* m_scrollLayer;
    CCLabelBMFont* m_contentLabel;
    CCLabelBMFont* m_pageLabel;

    bool init() {
        if (!Popup::init(380.f, 260.f)) return false;
        this->setTitle("Simulation History");

        loadFiles();

        auto bg = CCScale9Sprite::create("square02b_001.png");
        bg->setColor({0, 0, 0});
        bg->setOpacity(100);
        bg->setContentSize({300.0f, 160.0f});
        bg->setPosition({ m_mainLayer->getContentSize().width / 2, 135.0f });
        m_mainLayer->addChild(bg);

        m_scrollLayer = ScrollLayer::create({300.0f, 160.0f});
        m_scrollLayer->setPosition({ m_mainLayer->getContentSize().width / 2 - 150.0f, 55.0f });
        m_mainLayer->addChild(m_scrollLayer);

        m_contentLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_contentLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
        m_scrollLayer->m_contentLayer->addChild(m_contentLabel);

        auto menu = CCMenu::create();
        menu->setPosition({ m_mainLayer->getContentSize().width / 2, 135.0f });

        auto prevBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
            this, menu_selector(GDWTSessionViewer::onPrev)
        );
        prevBtn->setPosition({ -175.0f, 0.0f });
        menu->addChild(prevBtn);

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSprite->setFlipX(true);
        auto nextBtn = CCMenuItemSpriteExtra::create(
            nextSprite,
            this, menu_selector(GDWTSessionViewer::onNext)
        );
        nextBtn->setPosition({ 175.0f, 0.0f });
        menu->addChild(nextBtn);

        m_pageLabel = CCLabelBMFont::create("0 / 0", "chatFont.fnt");
        m_pageLabel->setScale(0.8f);
        m_pageLabel->setPosition({ 0.0f, -110.0f });
        menu->addChild(m_pageLabel);

        m_mainLayer->addChild(menu);

        updateDisplay();
        return true;
    }

    void loadFiles() {
        auto configDir = geode::Mod::get()->getConfigDir() / g_state.currentLevelKey;
        if (std::filesystem::exists(configDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(configDir)) {
                // FIX: Use generic_string() to safely read file names on all operating systems
                std::string filename = entry.path().filename().generic_string();
                if (filename.find("Simulation_") != std::string::npos) {
                    m_files.push_back(entry.path()); // Store path safely
                }
            }
        }
        std::sort(m_files.begin(), m_files.end());
        m_currentIndex = m_files.empty() ? -1 : 0;
    }

    void updateDisplay() {
        if (m_files.empty()) {
            m_contentLabel->setString("No sessions recorded yet.");
            m_pageLabel->setString("0 / 0");
            formatScrollText();
            return;
        }

        std::ifstream file(m_files[m_currentIndex]);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            m_contentLabel->setString(buffer.str().c_str());
            file.close();
        } else {
            m_contentLabel->setString("Error reading file.");
        }

        formatScrollText();
        m_pageLabel->setString(fmt::format("{} / {}", m_currentIndex + 1, m_files.size()).c_str());
    }

    void formatScrollText() {
        m_contentLabel->setScale(0.5f); 
        m_contentLabel->updateLabel(); 
        
        float labelHeight = m_contentLabel->getContentSize().height * 0.5f;
        float layerHeight = std::max(160.0f, labelHeight + 25.0f); 
        
        m_scrollLayer->m_contentLayer->setContentSize({300.0f, layerHeight});
        m_contentLabel->setPosition({150.0f, layerHeight - 10.0f});
        m_contentLabel->setAnchorPoint({0.5f, 1.0f});
        
        m_scrollLayer->moveToTop();
    }

    void onPrev(CCObject*) {
        if (m_currentIndex > 0) {
            m_currentIndex--;
            updateDisplay();
        }
    }

    void onNext(CCObject*) {
        if (m_currentIndex < static_cast<int>(m_files.size()) - 1) {
            m_currentIndex++;
            updateDisplay();
        }
    }

public:
    static GDWTSessionViewer* create() {
        auto ret = new GDWTSessionViewer();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// --- UI: Configuration Popup ---
class GDWTPopup : public geode::Popup { 
protected:
    TextInput* m_timeInput;
    TextInput* m_qualInput;
    TextInput* m_15Input;
    TextInput* m_20Input;

    bool init() { 
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

    void onStart(CCObject*) {
        float timeMin = utils::numFromString<float>(m_timeInput->getString()).unwrapOr(30.0f);
        g_state.qualifyPct = utils::numFromString<float>(m_qualInput->getString()).unwrapOr(32.0f);
        g_state.p15Pct = utils::numFromString<float>(m_15Input->getString()).unwrapOr(54.0f);
        g_state.p20Pct = utils::numFromString<float>(m_20Input->getString()).unwrapOr(75.0f);

        g_state.sessionDurationMinutes = timeMin;
        g_state.timeRemaining = timeMin * 60.0f;
        g_state.timeElapsed = 0.0f;
        g_state.totalPoints = 0.0f;
        g_state.currentPPM = 0.0f;
        g_state.allPercentages.clear();
        g_state.currentStreakCompletions = 0;
        g_state.bestStreakPct = 0;
        
        g_state.isPendingStart = true; 
        g_state.isActive = false;
        g_state.timeExpired = false;

        loadGlobalStats();

        FLAlertLayer::create("Simulation Ready", "Simulation configured! Hit Play to start.", "OK")->show();
        this->onClose(nullptr);
    }

    void onStats(CCObject*) { GDWTStatsPopup::create()->show(); }
    void onView(CCObject*) { GDWTSessionViewer::create()->show(); }

public:
    static GDWTPopup* create() {
        auto ret = new GDWTPopup();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// --- Level Info Screen Hook ---
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
        int id = this->m_level->m_levelID.value();
        
        std::string cleanName = "";
        for (char c : g_state.currentLevelName) {
            if (isalnum(c) || c == '-' || c == '_') cleanName += c;
            else if (c == ' ') cleanName += "_";
        }
        
        if (id > 0) g_state.currentLevelKey = fmt::format("{}_{}", id, cleanName);
        else g_state.currentLevelKey = cleanName.empty() ? "Local_Level" : cleanName;

        GDWTPopup::create()->show(); 
    }
};

// --- Gameplay Hook ---
class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_gdwtLabel = nullptr;
        CCLabelBMFont* m_paceLabel = nullptr; 
        bool m_hasScoredThisRun = false; 
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (g_state.isPendingStart) {
            g_state.isPendingStart = false;
            g_state.isActive = true;
        }

        if (g_state.isActive) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            m_fields->m_gdwtLabel = CCLabelBMFont::create("Points: 0 | 0:00", "bigFont.fnt");
            m_fields->m_gdwtLabel->setScale(0.4f);
            m_fields->m_gdwtLabel->setAnchorPoint({0, 1});
            m_fields->m_gdwtLabel->setPosition({ 5, winSize.height - 5 });
            m_fields->m_gdwtLabel->setOpacity(150);
            this->addChild(m_fields->m_gdwtLabel, 100);

            m_fields->m_paceLabel = CCLabelBMFont::create(
                fmt::format("Pace: {:.1f} PPM | Avg: {:.1f} | Best: {:.1f}", 
                    g_state.currentPPM, g_state.globalAvgPPM, g_state.globalBestPPM).c_str(), 
                "bigFont.fnt");
            m_fields->m_paceLabel->setScale(0.3f); 
            m_fields->m_paceLabel->setAnchorPoint({0, 1});
            m_fields->m_paceLabel->setPosition({ 5, winSize.height - 20 });
            m_fields->m_paceLabel->setColor({200, 200, 200}); 
            m_fields->m_paceLabel->setOpacity(150);
            this->addChild(m_fields->m_paceLabel, 100);
        }
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->m_hasScoredThisRun = false; 
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (g_state.isActive && !this->m_isPaused) {
            g_state.timeElapsed += dt; 

            if (!g_state.timeExpired) {
                g_state.timeRemaining -= dt;

                if (g_state.timeRemaining <= 0) {
                    g_state.timeRemaining = 0;
                    g_state.timeExpired = true; 
                }
            }

            if (m_fields->m_gdwtLabel && m_fields->m_paceLabel) {
                int mins = static_cast<int>(g_state.timeRemaining) / 60;
                int secs = static_cast<int>(g_state.timeRemaining) % 60;
                
                if (g_state.timeExpired) {
                    m_fields->m_gdwtLabel->setColor({255, 50, 50});
                }

                m_fields->m_gdwtLabel->setString(
                    fmt::format("GDWT: {:.1f} PTS | {}:{:02}", g_state.totalPoints, mins, secs).c_str()
                );
            }
        }
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_state.isActive && !this->m_level->isPlatformer() && !this->m_isPracticeMode && !m_fields->m_hasScoredThisRun) {
            
            int pct = static_cast<int>(this->getCurrentPercent());
            
            if (pct > 0 && pct < 100) {
                m_fields->m_hasScoredThisRun = true; 
                
                float pts = calculatePoints(static_cast<float>(pct));
                g_state.totalPoints += pts; 
                
                g_state.allPercentages.push_back(pct);
                
                int currentStreakScore = (g_state.currentStreakCompletions * 100) + pct;
                if (currentStreakScore > g_state.bestStreakPct) {
                    g_state.bestStreakPct = currentStreakScore;
                }
                g_state.currentStreakCompletions = 0;

                float elapsedMins = g_state.timeElapsed / 60.0f;
                g_state.currentPPM = (elapsedMins > 0.001f) ? (g_state.totalPoints / elapsedMins) : 0.0f;
                if (m_fields->m_paceLabel) {
                    m_fields->m_paceLabel->setString(
                        fmt::format("Pace: {:.1f} PPM | Avg: {:.1f} | Best: {:.1f}", 
                            g_state.currentPPM, g_state.globalAvgPPM, g_state.globalBestPPM).c_str()
                    );
                }

                if (g_state.timeExpired) {
                    processSessionEnd();
                    if (m_fields->m_gdwtLabel) m_fields->m_gdwtLabel->setVisible(false);
                    if (m_fields->m_paceLabel) m_fields->m_paceLabel->setVisible(false);
                }
            }
        }
        PlayLayer::destroyPlayer(p0, p1);
    }

    void levelComplete() {
        if (g_state.isActive && !this->m_isPracticeMode && !m_fields->m_hasScoredThisRun) {
            m_fields->m_hasScoredThisRun = true; 
            
            g_state.totalPoints += 250.0f; 

            g_state.allPercentages.push_back(100);
            g_state.currentStreakCompletions++;
            
            int currentStreakScore = g_state.currentStreakCompletions * 100;
            if (currentStreakScore > g_state.bestStreakPct) {
                g_state.bestStreakPct = currentStreakScore;
            }

            float elapsedMins = g_state.timeElapsed / 60.0f;
            g_state.currentPPM = (elapsedMins > 0.001f) ? (g_state.totalPoints / elapsedMins) : 0.0f;
            if (m_fields->m_paceLabel) {
                m_fields->m_paceLabel->setString(
                    fmt::format("Pace: {:.1f} PPM | Avg: {:.1f} | Best: {:.1f}", 
                        g_state.currentPPM, g_state.globalAvgPPM, g_state.globalBestPPM).c_str()
                );
            }

            if (g_state.timeExpired) {
                processSessionEnd();
                if (m_fields->m_gdwtLabel) m_fields->m_gdwtLabel->setVisible(false);
                if (m_fields->m_paceLabel) m_fields->m_paceLabel->setVisible(false);
            }
        }
        PlayLayer::levelComplete();
    }
};
