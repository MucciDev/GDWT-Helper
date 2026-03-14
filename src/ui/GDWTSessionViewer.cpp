#include "GDWTSessionViewer.hpp"
#include "../core/State.hpp"
#include <Geode/utils/string.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using namespace geode::prelude;

namespace {
    constexpr uintmax_t MAX_SESSION_FILE_SIZE = 1024 * 1024;
}

bool GDWTSessionViewer::init() {
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

void GDWTSessionViewer::loadFiles() {
    const auto configDir = geode::Mod::get()->getConfigDir() / g_state.currentLevelKey;
    std::error_code ec;
    const auto canonicalConfigDir = std::filesystem::weakly_canonical(configDir, ec);
    if (ec) return;

    if (std::filesystem::exists(configDir, ec) && !ec) {
        for (const auto& entry : std::filesystem::directory_iterator(configDir, ec)) {
            if (!entry.is_regular_file(ec) || ec) continue;

            const auto canonicalEntry = std::filesystem::weakly_canonical(entry.path(), ec);
            if (ec) continue;

            const auto relativePath = std::filesystem::relative(canonicalEntry, canonicalConfigDir, ec);
            if (ec || relativePath.empty() || relativePath.is_absolute()) continue;
            // Defense in depth: reject any path that still points outside the level config directory.
            if (relativePath.string().starts_with("..")) continue;

            const auto filename = geode::utils::string::pathToString(canonicalEntry.filename());
            if (filename.starts_with("Simulation_") && canonicalEntry.extension() == ".txt") {
                m_files.push_back(canonicalEntry);
            }
        }
    }
    std::sort(m_files.begin(), m_files.end());
    m_currentIndex = m_files.empty() ? -1 : 0;
}

void GDWTSessionViewer::updateDisplay() {
    if (m_files.empty()) {
        m_contentLabel->setString("No sessions recorded yet.");
        m_pageLabel->setString("0 / 0");
        formatScrollText();
        return;
    }

    std::error_code ec;
    const auto fileSize = std::filesystem::file_size(m_files[m_currentIndex], ec);
    if (ec || fileSize > MAX_SESSION_FILE_SIZE) {
        m_contentLabel->setString("Error reading file.");
        formatScrollText();
        m_pageLabel->setString(fmt::format("{} / {}", m_currentIndex + 1, (int)m_files.size()).c_str());
        return;
    }

    std::ifstream file(m_files[m_currentIndex], std::ios::in);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        m_contentLabel->setString(buffer.str().c_str());
        file.close();
    } else {
        m_contentLabel->setString("Error reading file.");
    }

    formatScrollText();
    m_pageLabel->setString(fmt::format("{} / {}", m_currentIndex + 1, (int)m_files.size()).c_str());
}

void GDWTSessionViewer::formatScrollText() {
    m_contentLabel->setScale(0.5f);
    m_contentLabel->updateLabel();

    const float labelHeight = m_contentLabel->getContentSize().height * 0.5f;
    const float layerHeight = std::max(160.0f, labelHeight + 25.0f);        

    m_scrollLayer->m_contentLayer->setContentSize({300.0f, layerHeight});   
    m_contentLabel->setPosition({150.0f, layerHeight - 10.0f});
    m_contentLabel->setAnchorPoint({0.5f, 1.0f});

    m_scrollLayer->moveToTop();
}

void GDWTSessionViewer::onPrev(CCObject*) {
    if (m_currentIndex > 0) {
        m_currentIndex--;
        updateDisplay();
    }
}

void GDWTSessionViewer::onNext(CCObject*) {
    if (m_currentIndex < static_cast<int>(m_files.size()) - 1) {
        m_currentIndex++;
        updateDisplay();
    }
}

GDWTSessionViewer* GDWTSessionViewer::create() {
    auto ret = new GDWTSessionViewer();
    if (ret && ret->init()) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
