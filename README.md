<p align="center">
  <img src="logo.png" width="150" alt="GDWT Helper Logo">
</p>

<h1 align="center">GDWT Helper</h1>

<p align="center">
  <img src="https://img.shields.io/github/v/release/MucciDev/GDWT-Helper?label=Latest%20Release&color=blue" alt="Latest Release">
  <img src="https://img.shields.io/github/downloads/MucciDev/GDWT-Helper/total?color=success" alt="Total Downloads">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20macOS-lightgrey" alt="Supported Platforms">
</p>

A Geometry Dash Geode mod designed to precisely simulate the Geometry Dash World Tournament (GDWT) point-tracking system. 

This mod allows you to configure custom time limits and percentage thresholds to calculate your points during a session. It tracks your attempts in real-time, calculates your Points Per Minute (PPM) pace, and automatically saves your run history for future review.

## Features

* **Custom Rulesets:** Manually set your session timer and percentage point thresholds (Qualifying, 1.5x, and 2.0x).
* **Live HUD:** Tracks your remaining time, current points, and your Current, Average, and Best PPM directly on-screen while you play.
* **Overtime Logic:** Just like the real GDWT, if the timer hits zero during an attempt, the run remains active until you crash or beat the level.
* **Persistent Stats:** Calculates and saves your lifetime Average PPM and Best PPM across all sessions for every specific level.
* **Session Logs:** Automatically generates a detailed text file in the Geode config folder after every session, containing your total score, best streak, and every single percentage you achieved.
* **In-Game Viewer:** Read your past session logs and analyze your grind directly inside the game.

## How to Use

1. Navigate to any level and click the **GDWT** button on the left side of the screen.
2. Enter your desired time limit and percentage thresholds.
3. Click **Ready**.
4. Play the level! The timer and point tracking will begin automatically as soon as you spawn in.

*(Note: Practice Mode runs and 0% immediate crashes are automatically ignored by the point tracker to prevent score inflation.)*

## Credits
* **Mod Development:** akaMelom
* **Logo Design:** LunarSpark (GOAT)