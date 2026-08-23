#include "../llviewerprecompiledheaders.h"
#include "ctsbodysettings.h"
#include "../llviewercontrol.h"

namespace
{
std::string nonEmptySetting(const char* setting, const char* fallback)
{
    const std::string value = gSavedPerAccountSettings.getString(setting);
    return value.empty() ? std::string(fallback) : value;
}
}

const std::vector<CTSBodySetting>& CTSBodySettings::knownBodies()
{
    static const std::vector<CTSBodySetting> values =
    {
        {"Lara X", "FSInventoryAssistantBodyLaraX"},
        {"Legacy", "FSInventoryAssistantBodyLegacy"},
        {"Reborn", "FSInventoryAssistantBodyReborn"},
        {"Lara", "FSInventoryAssistantBodyLara"},
        {"Lara Petite", "FSInventoryAssistantBodyLaraPetite"},
        {"Freya", "FSInventoryAssistantBodyFreya"},
        {"Hourglass", "FSInventoryAssistantBodyHourglass"},
        {"Physique", "FSInventoryAssistantBodyPhysique"},
        {"SLink Hourglass", "FSInventoryAssistantBodySLinkHourglass"},
        {"SLink Physique", "FSInventoryAssistantBodySLinkPhysique"},
        {"Kupra", "FSInventoryAssistantBodyKupra"},
        {"Gen X", "FSInventoryAssistantBodyGenX"}
    };
    return values;
}

bool CTSBodySettings::isEnabled(const std::string& bodyName)
{
    if (bodyName.empty())
        return true;

    for (const CTSBodySetting& body : knownBodies())
    {
        if (body.bodyName == bodyName)
            return gSavedPerAccountSettings.getBOOL(body.settingName);
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
            return gSavedPerAccountSettings.getBOOL(body.settingName);
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
        if (gSavedPerAccountSettings.getBOOL(body.settingName))
            result.push_back(body.bodyName);
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
