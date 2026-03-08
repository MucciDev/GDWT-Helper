# GDWT Helper

A Geometry Dash Geode mod designed to simulate the Geometry Dash World Tournament (GDWT) point-tracking system. 

This mod allows you to configure custom time limits and percentage thresholds to calculate your points during a session. It tracks your attempts in real-time, calculates your Points Per Minute (PPM) pace, and automatically saves your run history to a text file for review.

## Features

* **Custom Rulesets:** Manually set your session timer and percentage point thresholds (Qualifying, 1.5x, and 2.0x).
* **Live HUD:** Tracks your remaining time, current points, and your Current, Average, and Best PPM on-screen while you play.
* **Overtime Logic:** Just like the real GDWT, if the timer hits zero during an attempt, the run remains active until you crash or beat the level.
* **Persistent Stats:** Calculates and saves your lifetime Average PPM and Best PPM across all sessions.
* **Session Logs:** Automatically generates a detailed text file in the Geode config folder after every session, containing your total score, best streak, and every single percentage you achieved.
* **In-Game Viewer:** Read your past session logs directly inside the game.

## How to Use

1. Navigate to any level and click the **GDWT** button on the left side of the screen.
2. Enter your desired time limit and percentage thresholds.
3. Click **Ready**.
4. Play the level! The timer and point tracking will begin automatically as soon as you spawn in.