#include <Geode/modify/PlayLayer.hpp>
#include "../core/State.hpp"
#include "../core/Utils.hpp"

using namespace geode::prelude;

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
            const auto winSize = CCDirector::sharedDirector()->getWinSize();    

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

        // Guard clause to prevent updating inactive sessions
        if (!g_state.isActive || this->m_isPaused) return;

        g_state.timeElapsed += dt;

        if (!g_state.timeExpired) {
            g_state.timeRemaining -= dt;

            if (g_state.timeRemaining <= 0) {
                g_state.timeRemaining = 0;
                g_state.timeExpired = true;
            }
        }

        if (m_fields->m_gdwtLabel && m_fields->m_paceLabel) {
            const int mins = static_cast<int>(g_state.timeRemaining) / 60;      
            const int secs = static_cast<int>(g_state.timeRemaining) % 60;      

            if (g_state.timeExpired) {
                m_fields->m_gdwtLabel->setColor({255, 50, 50});
            }

            m_fields->m_gdwtLabel->setString(
                fmt::format("GDWT: {:.1f} PTS | {}:{:02}", g_state.totalPoints, mins, secs).c_str()
            );
        }
    }

    void processRunCompletion(int pct) {
        m_fields->m_hasScoredThisRun = true;

        const float pts = calculatePoints(static_cast<float>(pct));
        g_state.totalPoints += pts;
        g_state.allPercentages.push_back(pct);

        if (pct == 100) {
            g_state.currentStreakCompletions++;
        }

        const int currentStreakScore = (g_state.currentStreakCompletions * 100) + (pct == 100 ? 0 : pct);
        if (currentStreakScore > g_state.bestStreakPct) {
            g_state.bestStreakPct = currentStreakScore;
        }

        if (pct != 100) {
            g_state.currentStreakCompletions = 0;
        }

        const float elapsedMins = g_state.timeElapsed / SECONDS_PER_MINUTE;     
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

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_state.isActive && !this->m_level->isPlatformer() && !this->m_isPracticeMode && !m_fields->m_hasScoredThisRun) {
            const int pct = static_cast<int>(this->getCurrentPercent());        
            if (pct > 0 && pct < 100) {
                this->processRunCompletion(pct);
            }
        }
        PlayLayer::destroyPlayer(p0, p1);
    }

    void levelComplete() {
        if (g_state.isActive && !this->m_isPracticeMode && !m_fields->m_hasScoredThisRun) {
            this->processRunCompletion(100);
        }
        PlayLayer::levelComplete();
    }
};
