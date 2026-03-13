# Changelog

## v2.0.0
* Completely replaced the local file-saving system with a real-time HTTP ping system for global leaderboards and remote session tracking.
* Implemented SHA256 HMAC payload signatures to authenticate incoming percentage runs to the external backend database.
* Resolved Geode background crash (`c0000005`) by completely redesigning asynchronous networking to use stable, detached `std::thread` tasks.
* Fixed a gameplay hook issue where completing a level (100%) during an active session would occasionally fail to log the successful run to the server.

## v1.1.3
* Refactored the entire codebase to deeply implement the strict MVC architectural pattern.
* Cleanly decoupled all frontend UI components from backend core logic and game hooks, drastically improving project maintainability and performance.

## v1.1.2
* Replaced standard path string conversions with `geode::utils::pathToString()` to prevent crashes.
* Implemented `std::error_code` handling for directory creation to ensure the mod never throws C++ exceptions.
* Reinforced the use of `geode::utils::numFromString` with safe fallbacks to parse user input without risk of crashes.

## v1.1.1
* Added GitHub repository link to mod.json.
* Set up automated GitHub Actions release workflow.

## v1.1.0
* Changed file path generation to segregate stats and logs into subfolders based on the specific Level ID/Name.
* Fixed Cocos2d-x text truncation bug in the Session Viewer by removing manual setWidth and relying on automatic newlines.
* Replaced std::string casting with .generic_string() for filesystem paths to prevent crashes and comply with Geode Index guidelines.

## v1.0.2
* Added tags

## v1.0.1
* Added logo (W LunarSpark)

## v1.0.0
* Initial release.
* Added GDWT configuration popup to LevelInfoLayer.
* Added real-time point calculation and live timer HUD.
* Added persistent Points Per Minute (PPM) tracking.
* Added automatic text file generation for session histories.
* Added in-game Session Viewer and Lifetime Stats popups.