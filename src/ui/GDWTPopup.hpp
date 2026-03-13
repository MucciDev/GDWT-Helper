#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>

class GDWTPopup : public geode::Popup {
protected:
    geode::TextInput* m_timeInput;
    geode::TextInput* m_qualInput;
    geode::TextInput* m_15Input;
    geode::TextInput* m_20Input;

    bool init();
    void onStart(cocos2d::CCObject*);
    void onStats(cocos2d::CCObject*);
    void onView(cocos2d::CCObject*);

public:
    static GDWTPopup* create();
};
