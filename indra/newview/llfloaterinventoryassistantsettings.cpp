#include "llviewerprecompiledheaders.h"

#include "llfloaterinventoryassistantsettings.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "lllineeditor.h"
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
    {"FSInventoryAssistantBodyGenX", "body_genx"},
    {"FSInventoryAssistantHeadLeLutka", "head_lelutka"},
    {"FSInventoryAssistantHeadCATWA", "head_catwa"},
    {"FSInventoryAssistantHeadGENUS", "head_genus"},
    {"FSInventoryAssistantHeadLAQ", "head_laq"},
    {"FSInventoryAssistantTargetUnrigged", "target_unrigged"}
};

std::string nonEmpty(const std::string& value, const char* fallback)
{
    return value.empty() ? std::string(fallback) : value;
}
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

    getChild<LLCheckBoxCtrl>("split_main_folders")->setValue(
        gSavedPerAccountSettings.getBOOL("FSInventoryAssistantSplitMainFolders"));
    getChild<LLCheckBoxCtrl>("split_hud_folder")->setValue(
        gSavedPerAccountSettings.getBOOL("FSInventoryAssistantSplitHUDFolder"));
    getChild<LLCheckBoxCtrl>("split_alpha_folder")->setValue(
        gSavedPerAccountSettings.getBOOL("FSInventoryAssistantSplitAlphaFolder"));

    getChild<LLLineEditor>("hud_folder_name")->setText(
        nonEmpty(gSavedPerAccountSettings.getString("FSInventoryAssistantHUDFolderName"), "HUD"));
    getChild<LLLineEditor>("alpha_folder_name")->setText(
        nonEmpty(gSavedPerAccountSettings.getString("FSInventoryAssistantAlphaFolderName"), "Alpha"));
    getChild<LLLineEditor>("addon_folder_name")->setText(
        nonEmpty(gSavedPerAccountSettings.getString("FSInventoryAssistantAddOnFolderName"), "Add On"));
}

void LLFloaterInventoryAssistantSettings::saveSettings()
{
    for (const BodyControl& body : BODY_CONTROLS)
    {
        gSavedPerAccountSettings.setBOOL(
            body.setting,
            getChild<LLCheckBoxCtrl>(body.control)->getValue().asBoolean());
    }

    gSavedPerAccountSettings.setBOOL(
        "FSInventoryAssistantSplitMainFolders",
        getChild<LLCheckBoxCtrl>("split_main_folders")->getValue().asBoolean());
    gSavedPerAccountSettings.setBOOL(
        "FSInventoryAssistantSplitHUDFolder",
        getChild<LLCheckBoxCtrl>("split_hud_folder")->getValue().asBoolean());
    gSavedPerAccountSettings.setBOOL(
        "FSInventoryAssistantSplitAlphaFolder",
        getChild<LLCheckBoxCtrl>("split_alpha_folder")->getValue().asBoolean());

    gSavedPerAccountSettings.setString(
        "FSInventoryAssistantHUDFolderName",
        nonEmpty(getChild<LLLineEditor>("hud_folder_name")->getText(), "HUD"));
    gSavedPerAccountSettings.setString(
        "FSInventoryAssistantAlphaFolderName",
        nonEmpty(getChild<LLLineEditor>("alpha_folder_name")->getText(), "Alpha"));
    gSavedPerAccountSettings.setString(
        "FSInventoryAssistantAddOnFolderName",
        nonEmpty(getChild<LLLineEditor>("addon_folder_name")->getText(), "Add On"));
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
