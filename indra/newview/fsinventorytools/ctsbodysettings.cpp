#include "../llviewerprecompiledheaders.h"

#include "ctsbodysettings.h"
#include "../llviewercontrol.h"

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
