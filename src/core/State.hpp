#pragma once

#include <vector>
#include <string>

// --- Constants ---
constexpr float POINTS_FOR_COMPLETION = 250.0f;
constexpr float SECONDS_PER_MINUTE = 60.0f;
constexpr float DEFAULT_TIMER_MINUTES = 30.0f;

// --- Global State ---
struct GDWTState {
    bool isPendingStart = false; 
    bool isActive = false;
    bool timeExpired = false;

    // Level identification
    std::string currentLevelKey = "Unknown";
    std::string currentLevelName = "Level";

    // Session timers
    float timeRemaining = 0.0f;
    float timeElapsed = 0.0f;
    float sessionDurationMinutes = 0.0f;

    // Scoring thresholds
    float qualifyPct = 25.0f;
    float p15Pct = 50.0f;
    float p20Pct = 75.0f;

    // Session stats
    float totalPoints = 0.0f;
    float currentPPM = 0.0f;
    std::vector<int> allPercentages;
    int currentStreakCompletions = 0;
    int bestStreakPct = 0;

    // Lifetime stats
    float globalBestPPM = 0.0f;
    float globalAvgPPM = 0.0f;
    float globalTotalMins = 0.0f;
};

inline GDWTState g_state;
