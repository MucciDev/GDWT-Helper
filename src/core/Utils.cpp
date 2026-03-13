#include "Utils.hpp"
#include "State.hpp"
#include <Geode/Geode.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <system_error>
#include <fstream>

using namespace geode::prelude;

std::string getCurrentDateString() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(now);        
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

void loadGlobalStats() {
    const auto configDir = geode::Mod::get()->getConfigDir() / g_state.currentLevelKey;
    const auto statsPath = configDir / "stats.txt";

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
    if (pct >= 100.0f) return POINTS_FOR_COMPLETION;
    if (pct >= g_state.p20Pct) return pct * 2.0f;
    if (pct >= g_state.p15Pct) return pct * 1.5f;
    if (pct >= g_state.qualifyPct) return pct;
    return 0.0f;
}

void processSessionEnd() {
    g_state.isActive = false;

    const float elapsedMins = g_state.timeElapsed / SECONDS_PER_MINUTE;
    const float finalPPM = (elapsedMins > 0.0f) ? (g_state.totalPoints / elapsedMins) : 0.0f;

    // Format run history string
    std::string pctString = "";
    for (size_t i = 0; i < g_state.allPercentages.size(); ++i) {
        pctString += std::to_string(g_state.allPercentages[i]) + "%";
        if (i < g_state.allPercentages.size() - 1) {
            pctString += ", ";
            if ((i + 1) % 10 == 0) pctString += "\n";
        }
    }
    if (pctString.empty()) pctString = "No valid runs completed.";

    const auto configDir = geode::Mod::get()->getConfigDir() / g_state.currentLevelKey;

    // Ensure directory exists without throwing exceptions
    std::error_code ec;
    std::filesystem::create_directories(configDir, ec);

    // Save current session log
    const std::string dateStr = getCurrentDateString();
    const auto runPath = configDir / fmt::format("Simulation_{}.txt", dateStr); 

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

    // Read existing lifetime stats to append new data
    float bestPPM = 0.0f, totalHistoricalPts = 0.0f, totalHistoricalMins = 0.0f;
    const auto statsPath = configDir / "stats.txt";
    
    std::ifstream inFile(statsPath);
    if (inFile.is_open()) {
        inFile >> bestPPM >> totalHistoricalPts >> totalHistoricalMins;
        inFile.close();
    }

    if (finalPPM > bestPPM) bestPPM = finalPPM;
    totalHistoricalPts += g_state.totalPoints;
    totalHistoricalMins += elapsedMins;

    // Overwrite with updated lifetime stats
    std::ofstream outFile(statsPath);
    if (outFile.is_open()) {
        outFile << bestPPM << " " << totalHistoricalPts << " " << totalHistoricalMins;
        outFile.close();
    }

    // Format output for UI alert
    std::string uiPctString = pctString;
    if (uiPctString.length() > 80) {
        uiPctString = uiPctString.substr(0, 77) + "...";
    }

    const auto msg = fmt::format(
        "Total Points: {:.1f}\nPace: {:.1f} PPM\nBest Streak: {}%\n\nLog saved to config folder!\n\nHistory:\n{}",
        g_state.totalPoints,
        finalPPM,
        g_state.bestStreakPct,
        uiPctString
    );

    FLAlertLayer::create("Simulation Finished", msg, "OK")->show();
}
