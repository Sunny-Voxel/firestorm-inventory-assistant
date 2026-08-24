#ifndef CTS_BODY_SETTINGS_H
#define CTS_BODY_SETTINGS_H

#include <string>
#include <vector>

enum class CTSBodySex
{
    Any,
    Female,
    Male
};

struct CTSBodySetting
{
    std::string bodyName;
    std::string settingName;
    CTSBodySex sex = CTSBodySex::Any;
};

class CTSBodySettings
{
public:
    static const std::vector<CTSBodySetting>& knownBodies();
    static bool isEnabled(const std::string& bodyName);
    static bool isTargetEnabled(const std::string& targetName);
    static std::vector<std::string> enabledBodies();
    static bool autoBodyGender();
    static bool bodyMatchesAvatar(const CTSBodySetting& body);
    static const std::vector<CTSBodySetting>& knownHeads();
    static std::vector<std::string> enabledHeads();
    static std::vector<std::string> enabledOtherTargets();

    static bool splitMainFolders();
    static bool splitHUDFolder();
    static bool splitAlphaFolder();

    static std::string hudFolderName();
    static std::string alphaFolderName();
    static std::string addOnFolderName();

    // Folder naming is deliberately separate from classifier target names.
    // mode: "cts", "descriptive" or "custom".
    static std::string folderNamingMode();
    static std::string descriptiveSuffix();
    static std::string customTargetFolderName(const std::string& targetName);
    static std::string outputTargetFolderName(const std::string& targetName);
};

#endif
