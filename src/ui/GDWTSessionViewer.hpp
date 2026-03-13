#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <vector>
#include <filesystem>

class GDWTSessionViewer : public geode::Popup {
protected:
    std::vector<std::filesystem::path> m_files;
    int m_currentIndex = 0;

    geode::ScrollLayer* m_scrollLayer;
    cocos2d::CCLabelBMFont* m_contentLabel;
    cocos2d::CCLabelBMFont* m_pageLabel;

    bool init();
    void loadFiles();
    void updateDisplay();
    void formatScrollText();
    void onPrev(cocos2d::CCObject*);
    void onNext(cocos2d::CCObject*);

public:
    static GDWTSessionViewer* create();
};
