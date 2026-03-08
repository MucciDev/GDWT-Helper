# Changelog

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