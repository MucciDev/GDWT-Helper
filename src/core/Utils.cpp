#include "Utils.hpp"
#include "State.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <bit>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>
#include <vector>

using namespace geode::prelude;

#pragma optimize("", off)

constexpr auto SERVER_URL_SECURE = "https://gdwt.alwaysdata.net/index.php?api=ping";
constexpr auto MOD_AUTH_KEY  = "AuraPlusEgoEqualDonPolloLayoutVerifiedByZoink67";
constexpr auto SECRET_SALT   = "FemboySparkIsNotAFemboy123";

namespace {
    std::string sha256Hex(const std::string& input) {
        uint32_t H[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };
        const uint32_t K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        std::vector<uint8_t> data(input.begin(), input.end());
        uint64_t bitlen = data.size() * 8;
        data.push_back(0x80);
        while ((data.size() % 64) != 56) data.push_back(0);
        for (int i = 7; i >= 0; --i) data.push_back(static_cast<uint8_t>(bitlen >> (i * 8)));

        for (size_t offset = 0; offset < data.size(); offset += 64) {
            uint32_t w[64], a, b, c, d, e, f, g, h;
            for (int i = 0; i < 16; ++i) {
                w[i] = (data[offset + i * 4] << 24) | (data[offset + i * 4 + 1] << 16) |
                       (data[offset + i * 4 + 2] << 8) | (data[offset + i * 4 + 3]);
            }
            for (int i = 16; i < 64; ++i) {
                uint32_t s0 = std::rotl(w[i - 15], 32 - 7) ^ std::rotl(w[i - 15], 32 - 18) ^ (w[i - 15] >> 3);
                uint32_t s1 = std::rotl(w[i - 2], 32 - 17) ^ std::rotl(w[i - 2], 32 - 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            a = H[0]; b = H[1]; c = H[2]; d = H[3];
            e = H[4]; f = H[5]; g = H[6]; h = H[7];

            for (int i = 0; i < 64; ++i) {
                uint32_t S1 = std::rotl(e, 32 - 6) ^ std::rotl(e, 32 - 11) ^ std::rotl(e, 32 - 25);
                uint32_t ch = (e & f) ^ (~e & g);
                uint32_t temp1 = h + S1 + ch + K[i] + w[i];
                uint32_t S0 = std::rotl(a, 32 - 2) ^ std::rotl(a, 32 - 13) ^ std::rotl(a, 32 - 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = S0 + maj;

                h = g; g = f; f = e; e = d + temp1;
                d = c; c = b; b = a; a = temp1 + temp2;
            }

            H[0] += a; H[1] += b; H[2] += c; H[3] += d;
            H[4] += e; H[5] += f; H[6] += g; H[7] += h;
        }

        std::stringstream ss;
        for (int i = 0; i < 8; ++i) {
            ss << std::hex << std::setw(8) << std::setfill('0') << H[i];
        }
        return ss.str();
    }

    std::string buildPingHash(const std::string& action, const int accountID, const std::string& levelKey) {
        const auto source = action + std::to_string(accountID) + levelKey + SECRET_SALT;
        return sha256Hex(source);
    }

void sendPingAsync(matjson::Value payload, std::string action) {
        std::thread([payload = std::move(payload), action = std::move(action)]() {
            geode::utils::thread::setName("GDWT Web Request");
            const auto makeRequest = [&payload]() {
                auto request = geode::utils::web::WebRequest();
                request
                    .header("X-GDWT-Auth", MOD_AUTH_KEY)
                    .header("Content-Type", "application/json")
                    .bodyJSON(payload)
                    .timeout(std::chrono::seconds(10));
                return request;
            };

            auto response = makeRequest().postSync(SERVER_URL_SECURE);

            if (!response.ok()) {
                log::error("[GDWT] Server ping '{}' failed. HTTP {} | {}", action, response.code(), response.errorMessage());
            } else {
                log::info("[GDWT] Server ping '{}' succeeded (HTTP {})", action, response.code());
            }
        }).detach();
    }
}

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

    float bestPPM = 0.0f;
    float totalPts = 0.0f;
    float totalMins = 0.0f;

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

void sendServerPing(const std::string& action, const int pct, const float points, const float ppm) {
    if (action != "start" && action != "crash" && action != "finish") {
        log::warn("[GDWT] Ignored unsupported ping action: {}", action);
        return;
    }

    auto* const accountManager = GJAccountManager::sharedState();
    if (accountManager == nullptr) {
        log::error("[GDWT] Failed to send '{}' ping: missing account manager", action);
        return;
    }

    const int accountID = accountManager->m_accountID;
    const std::string username = accountManager->m_username.empty() ? "Unknown" : accountManager->m_username;
    const std::string levelKey = g_state.currentLevelKey.empty() ? "Unknown_Level" : g_state.currentLevelKey;
    const std::string levelName = g_state.currentLevelName.empty() ? "Unknown" : g_state.currentLevelName;
    const std::string hash = buildPingHash(action, accountID, levelKey);

    matjson::Value payload;
    if (action == "crash") {
        payload = matjson::makeObject({
            {"action", action},
            {"account_id", accountID},
            {"username", username},
            {"level_key", levelKey},
            {"level_name", levelName},
            {"hash", hash},
            {"pct", pct},
            {"points", points}
        });
    }
    else if (action == "finish") {
        payload = matjson::makeObject({
            {"action", action},
            {"account_id", accountID},
            {"username", username},
            {"level_key", levelKey},
            {"level_name", levelName},
            {"hash", hash},
            {"ppm", ppm}
        });
    }
    else {
        payload = matjson::makeObject({
            {"action", action},
            {"account_id", accountID},
            {"username", username},
            {"level_key", levelKey},
            {"level_name", levelName},
            {"hash", hash}
        });
    }

    sendPingAsync(payload, action);
}

void processSessionEnd() {
    g_state.isActive = false;

    const float elapsedMins = g_state.timeElapsed / SECONDS_PER_MINUTE;
    const float finalPPM = (elapsedMins > 0.0f) ? (g_state.totalPoints / elapsedMins) : 0.0f;

    sendServerPing("finish", 0, 0.0f, finalPPM);

    std::string pctString;
    for (size_t i = 0; i < g_state.allPercentages.size(); ++i) {
        pctString += std::to_string(g_state.allPercentages[i]) + "%";
        if (i < g_state.allPercentages.size() - 1) {
            pctString += ", ";
            if ((i + 1) % 10 == 0) {
                pctString += "\n";
            }
        }
    }
    if (pctString.empty()) {
        pctString = "No valid runs completed.";
    }

    const auto configDir = geode::Mod::get()->getConfigDir() / g_state.currentLevelKey;

    std::error_code ec;
    std::filesystem::create_directories(configDir, ec);

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
