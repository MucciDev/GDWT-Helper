#pragma once

#include <Geode/Geode.hpp>

class GDWTStatsPopup : public geode::Popup {
protected:
    bool init();
public:
    static GDWTStatsPopup* create();
};
