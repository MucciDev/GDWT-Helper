#pragma once

#include <string>

std::string getCurrentDateString();
void loadGlobalStats();
float calculatePoints(float pct);
void sendServerPing(const std::string& action, int pct = 0, float points = 0.0f, float ppm = 0.0f);
void processSessionEnd();
