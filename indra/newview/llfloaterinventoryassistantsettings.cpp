#include "llviewerprecompiledheaders.h"

#include "llfloaterinventoryassistantsettings.h"

#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llviewercontrol.h"

namespace
{
struct BodyControl
{
    const char* setting;
    const char* control;
};

const BodyControl BODY_CONTROLS[] =
{
    {"FSInventoryAssistantBodyLaraX", "body_larax"},
    {"FSInventoryAssistantBodyLegacy", "body_legacy"},
    {"FSInventoryAssistantBodyReborn", "body_reborn"},
    {"FSInventoryAssistantBodyLara", "body_lara"},
    {"FSInventoryAssistantBodyLaraPetite", "body_larapetite"},
    {"FSInventoryAssistantBodyFreya", "body_freya"},
    {"FSInventoryAssistantBodyHourglass", "body_hourglass"},
    {"FSInventoryAssistantBodyPhysique", "body_physique"},
    {"FSInventoryAssistantBodySLinkHourglass", "body_slinkhourglass"},
    {"FSInventoryAssistantBodySLinkPhysique", "body_slinkphysique"},
    {"FSInventoryAssistantBodyKupra", "body_kupra"},
    {"FSInventoryAssistantBodyGenX", "body_genx"}
};
}

LLFloaterInventoryAssistantSettings::
LLFloaterInventoryAssistantSettings(const LLSD& key)
    : LLFloater(key)
{
}

bool LLFloaterInventoryAssistantSettings::postBuild()
{
    getChild<LLButton>("save_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistantSettings::onSaveClicked, this));
    getChild<LLButton>("cancel_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistantSettings::onCancelClicked, this));

    loadSettings();
    return true;
}

void LLFloaterInventoryAssistantSettings::onOpen(const LLSD&)
{
    loadSettings();
}

void LLFloaterInventoryAssistantSettings::loadSettings()
{
    for (const BodyControl& body : BODY_CONTROLS)
    {
        getChild<LLCheckBoxCtrl>(body.control)->setValue(
            gSavedPerAccountSettings.getBOOL(body.setting));
    }
}

void LLFloaterInventoryAssistantSettings::saveSettings()
{
    for (const BodyControl& body : BODY_CONTROLS)
    {
        gSavedPerAccountSettings.setBOOL(
            body.setting,
            getChild<LLCheckBoxCtrl>(body.control)->getValue().asBoolean());
    }
}

void LLFloaterInventoryAssistantSettings::onSaveClicked()
{
    saveSettings();
    closeFloater();
}

void LLFloaterInventoryAssistantSettings::onCancelClicked()
{
    closeFloater();
}
