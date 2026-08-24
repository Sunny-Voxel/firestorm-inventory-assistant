#include "../llviewerprecompiledheaders.h"
#include "ctsbodysettings.h"
#include "../llviewercontrol.h"
#include "../llvoavatarself.h"

namespace
{
std::string nonEmptySetting(const char* setting, const char* fallback)
{
    const std::string value = gSavedPerAccountSettings.getString(setting);
    return value.empty() ? std::string(fallback) : value;
}

std::string customSettingName(const std::string& targetName)
{
    if (targetName == "Lara X") return "FSInventoryAssistantFolderNameLaraX";
    if (targetName == "Legacy") return "FSInventoryAssistantFolderNameLegacy";
    if (targetName == "Reborn") return "FSInventoryAssistantFolderNameReborn";
    if (targetName == "Lara") return "FSInventoryAssistantFolderNameLara";
    if (targetName == "Lara Petite") return "FSInventoryAssistantFolderNameLaraPetite";
    if (targetName == "Freya") return "FSInventoryAssistantFolderNameFreya";
    if (targetName == "Hourglass") return "FSInventoryAssistantFolderNameHourglass";
    if (targetName == "Physique") return "FSInventoryAssistantFolderNamePhysique";
    if (targetName == "SLink Hourglass") return "FSInventoryAssistantFolderNameSLinkHourglass";
    if (targetName == "SLink Physique") return "FSInventoryAssistantFolderNameSLinkPhysique";
    if (targetName == "Kupra") return "FSInventoryAssistantFolderNameKupra";
    if (targetName == "Gen X") return "FSInventoryAssistantFolderNameGenX";
    if (targetName == "Erika") return "FSInventoryAssistantFolderNameErika";
    if (targetName == "Legacy Male") return "FSInventoryAssistantFolderNameLegacyMale";
    if (targetName == "Jake") return "FSInventoryAssistantFolderNameJake";
    if (targetName == "Gianni") return "FSInventoryAssistantFolderNameGianni";
    if (targetName == "Kario") return "FSInventoryAssistantFolderNameKario";
    if (targetName == "LeLutka") return "FSInventoryAssistantFolderNameLeLutka";
    if (targetName == "CATWA") return "FSInventoryAssistantFolderNameCATWA";
    if (targetName == "GENUS") return "FSInventoryAssistantFolderNameGENUS";
    if (targetName == "LAQ") return "FSInventoryAssistantFolderNameLAQ";
    if (targetName == "Unrigged") return "FSInventoryAssistantFolderNameUnrigged";
    return std::string();
}
}

const std::vector<CTSBodySetting>& CTSBodySettings::knownBodies()
{
    static const std::vector<CTSBodySetting> values =
    {
        {"Lara X", "FSInventoryAssistantBodyLaraX", CTSBodySex::Female},
        {"Legacy", "FSInventoryAssistantBodyLegacy", CTSBodySex::Any},
        {"Reborn", "FSInventoryAssistantBodyReborn", CTSBodySex::Female},
        {"Lara", "FSInventoryAssistantBodyLara", CTSBodySex::Female},
        {"Lara Petite", "FSInventoryAssistantBodyLaraPetite", CTSBodySex::Female},
        {"Freya", "FSInventoryAssistantBodyFreya", CTSBodySex::Female},
        {"Hourglass", "FSInventoryAssistantBodyHourglass", CTSBodySex::Female},
        {"Physique", "FSInventoryAssistantBodyPhysique", CTSBodySex::Female},
        {"SLink Hourglass", "FSInventoryAssistantBodySLinkHourglass", CTSBodySex::Female},
        {"SLink Physique", "FSInventoryAssistantBodySLinkPhysique", CTSBodySex::Female},
        {"Kupra", "FSInventoryAssistantBodyKupra", CTSBodySex::Female},
        {"Gen X", "FSInventoryAssistantBodyGenX", CTSBodySex::Female},
        {"Erika", "FSInventoryAssistantBodyErika", CTSBodySex::Female},
        {"Legacy Male", "FSInventoryAssistantBodyLegacyMale", CTSBodySex::Male},
        {"Jake", "FSInventoryAssistantBodyJake", CTSBodySex::Male},
        {"Gianni", "FSInventoryAssistantBodyGianni", CTSBodySex::Male},
        {"Kario", "FSInventoryAssistantBodyKario", CTSBodySex::Male}
    };
    return values;
}

bool CTSBodySettings::autoBodyGender()
{
    return gSavedPerAccountSettings.getBOOL(
        "FSInventoryAssistantAutoBodyGender");
}

bool CTSBodySettings::bodyMatchesAvatar(const CTSBodySetting& body)
{
    if (!autoBodyGender() || body.sex == CTSBodySex::Any || !gAgentAvatarp)
        return true;

    const ESex avatarSex = gAgentAvatarp->getSex();
    if (body.sex == CTSBodySex::Male)
        return avatarSex == SEX_MALE;
    if (body.sex == CTSBodySex::Female)
        return avatarSex == SEX_FEMALE;

    return true;
}

bool CTSBodySettings::isEnabled(const std::string& bodyName)
{
    if (bodyName.empty())
        return true;

    for (const CTSBodySetting& body : knownBodies())
    {
        if (body.bodyName == bodyName)
            return gSavedPerAccountSettings.getBOOL(body.settingName) &&
                   bodyMatchesAvatar(body);
    }
    return false;
}


bool CTSBodySettings::isTargetEnabled(const std::string& targetName)
{
    if (targetName.empty())
        return true;

    for (const CTSBodySetting& body : knownBodies())
    {
        if (body.bodyName == targetName)
            return gSavedPerAccountSettings.getBOOL(body.settingName) &&
                   bodyMatchesAvatar(body);
    }

    for (const CTSBodySetting& head : knownHeads())
    {
        if (head.bodyName == targetName)
            return gSavedPerAccountSettings.getBOOL(head.settingName);
    }

    if (targetName == "Unrigged")
        return gSavedPerAccountSettings.getBOOL(
            "FSInventoryAssistantTargetUnrigged");

    return false;
}

std::vector<std::string> CTSBodySettings::enabledBodies()
{
    std::vector<std::string> result;
    for (const CTSBodySetting& body : knownBodies())
    {
        if (gSavedPerAccountSettings.getBOOL(body.settingName) &&
            bodyMatchesAvatar(body))
        {
            result.push_back(body.bodyName);
        }
    }
    return result;
}


const std::vector<CTSBodySetting>& CTSBodySettings::knownHeads()
{
    static const std::vector<CTSBodySetting> values =
    {
        {"LeLutka", "FSInventoryAssistantHeadLeLutka"},
        {"CATWA", "FSInventoryAssistantHeadCATWA"},
        {"GENUS", "FSInventoryAssistantHeadGENUS"},
        {"LAQ", "FSInventoryAssistantHeadLAQ"}
    };
    return values;
}

std::vector<std::string> CTSBodySettings::enabledHeads()
{
    std::vector<std::string> result;
    for (const CTSBodySetting& head : knownHeads())
        if (gSavedPerAccountSettings.getBOOL(head.settingName))
            result.push_back(head.bodyName);
    return result;
}


std::vector<std::string> CTSBodySettings::enabledOtherTargets()
{
    std::vector<std::string> result;
    if (gSavedPerAccountSettings.getBOOL(
            "FSInventoryAssistantTargetUnrigged"))
    {
        result.push_back("Unrigged");
    }
    return result;
}

bool CTSBodySettings::splitMainFolders()
{
    return gSavedPerAccountSettings.getBOOL("FSInventoryAssistantSplitMainFolders");
}

bool CTSBodySettings::splitHUDFolder()
{
    return gSavedPerAccountSettings.getBOOL("FSInventoryAssistantSplitHUDFolder");
}

bool CTSBodySettings::splitAlphaFolder()
{
    return gSavedPerAccountSettings.getBOOL("FSInventoryAssistantSplitAlphaFolder");
}

std::string CTSBodySettings::hudFolderName()
{
    return nonEmptySetting("FSInventoryAssistantHUDFolderName", "HUD");
}

std::string CTSBodySettings::alphaFolderName()
{
    return nonEmptySetting("FSInventoryAssistantAlphaFolderName", "Alpha");
}

std::string CTSBodySettings::addOnFolderName()
{
    return nonEmptySetting("FSInventoryAssistantAddOnFolderName", "Add On");
}

std::string CTSBodySettings::folderNamingMode()
{
    const std::string mode =
        gSavedPerAccountSettings.getString("FSInventoryAssistantFolderNamingMode");
    if (mode == "descriptive" || mode == "custom")
        return mode;
    return "cts";
}

std::string CTSBodySettings::descriptiveSuffix()
{
    return nonEmptySetting("FSInventoryAssistantDescriptiveSuffix", "Item");
}

std::string CTSBodySettings::customTargetFolderName(const std::string& targetName)
{
    const std::string setting = customSettingName(targetName);
    if (setting.empty())
        return targetName;

    const std::string value = gSavedPerAccountSettings.getString(setting);
    return value.empty() ? targetName : value;
}

std::string CTSBodySettings::outputTargetFolderName(const std::string& targetName)
{
    const std::string mode = folderNamingMode();

    if (mode == "custom")
        return customTargetFolderName(targetName);

    if (mode == "descriptive")
        return targetName + " " + descriptiveSuffix();

    return targetName;
}
