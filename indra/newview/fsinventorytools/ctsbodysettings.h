#ifndef CTS_BODY_SETTINGS_H
#define CTS_BODY_SETTINGS_H

#include <string>
#include <vector>

struct CTSBodySetting
{
    std::string bodyName;
    std::string settingName;
};

class CTSBodySettings
{
public:
    static const std::vector<CTSBodySetting>& knownBodies();
    static bool isEnabled(const std::string& bodyName);
    static bool isTargetEnabled(const std::string& targetName);
    static std::vector<std::string> enabledBodies();
    static const std::vector<CTSBodySetting>& knownHeads();
    static std::vector<std::string> enabledHeads();
    static std::vector<std::string> enabledOtherTargets();

    static bool splitMainFolders();
    static bool splitHUDFolder();
    static bool splitAlphaFolder();

    static std::string hudFolderName();
    static std::string alphaFolderName();
    static std::string addOnFolderName();
};

#endif
