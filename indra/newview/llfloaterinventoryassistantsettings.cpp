#include "llviewerprecompiledheaders.h"

#include "llfloaterinventoryassistantsettings.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "lllineeditor.h"
#include "llradiogroup.h"
#include "llviewercontrol.h"

namespace
{
struct BodyControl
{
    const char* setting;
    const char* control;
};

struct FolderNameControl
{
    const char* setting;
    const char* control;
    const char* fallback;
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
    {"FSInventoryAssistantBodyErika", "body_erika"},
    {"FSInventoryAssistantBodyLegacyMale", "body_legacymale"},
    {"FSInventoryAssistantBodyJake", "body_jake"},
    {"FSInventoryAssistantBodyGianni", "body_gianni"},
    {"FSInventoryAssistantBodyKario", "body_kario"},
    {"FSInventoryAssistantHeadLeLutka", "head_lelutka"},
    {"FSInventoryAssistantHeadCATWA", "head_catwa"},
    {"FSInventoryAssistantHeadGENUS", "head_genus"},
    {"FSInventoryAssistantHeadLAQ", "head_laq"},
    {"FSInventoryAssistantTargetUnrigged", "target_unrigged"}
};

const FolderNameControl FOLDER_NAME_CONTROLS[] =
{
    {"FSInventoryAssistantFolderNameLaraX", "folder_larax", "Lara X"},
    {"FSInventoryAssistantFolderNameLegacy", "folder_legacy", "Legacy"},
    {"FSInventoryAssistantFolderNameReborn", "folder_reborn", "Reborn"},
    {"FSInventoryAssistantFolderNameLara", "folder_lara", "Lara"},
    {"FSInventoryAssistantFolderNameLaraPetite", "folder_larapetite", "Lara Petite"},
    {"FSInventoryAssistantFolderNameFreya", "folder_freya", "Freya"},
    {"FSInventoryAssistantFolderNameHourglass", "folder_hourglass", "Hourglass"},
    {"FSInventoryAssistantFolderNamePhysique", "folder_physique", "Physique"},
    {"FSInventoryAssistantFolderNameSLinkHourglass", "folder_slinkhourglass", "SLink Hourglass"},
    {"FSInventoryAssistantFolderNameSLinkPhysique", "folder_slinkphysique", "SLink Physique"},
    {"FSInventoryAssistantFolderNameKupra", "folder_kupra", "Kupra"},
    {"FSInventoryAssistantFolderNameGenX", "folder_genx", "Gen X"},
    {"FSInventoryAssistantFolderNameErika", "folder_erika", "Erika"},
    {"FSInventoryAssistantFolderNameLegacyMale", "folder_legacymale", "Legacy Male"},
    {"FSInventoryAssistantFolderNameJake", "folder_jake", "Jake"},
    {"FSInventoryAssistantFolderNameGianni", "folder_gianni", "Gianni"},
    {"FSInventoryAssistantFolderNameKario", "folder_kario", "Kario"},
    {"FSInventoryAssistantFolderNameLeLutka", "folder_lelutka", "LeLutka"},
    {"FSInventoryAssistantFolderNameCATWA", "folder_catwa", "CATWA"},
    {"FSInventoryAssistantFolderNameGENUS", "folder_genus", "GENUS"},
    {"FSInventoryAssistantFolderNameLAQ", "folder_laq", "LAQ"},
    {"FSInventoryAssistantFolderNameUnrigged", "folder_unrigged", "Unrigged"}
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

    getChild<LLCheckBoxCtrl>("auto_body_gender")->setValue(
        gSavedPerAccountSettings.getBOOL("FSInventoryAssistantAutoBodyGender"));

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

    std::string namingMode =
        gSavedPerAccountSettings.getString("FSInventoryAssistantFolderNamingMode");
    if (namingMode != "descriptive" && namingMode != "custom")
        namingMode = "cts";
    getChild<LLRadioGroup>("folder_naming_mode")->setValue(namingMode);

    getChild<LLLineEditor>("descriptive_suffix")->setText(
        nonEmpty(gSavedPerAccountSettings.getString("FSInventoryAssistantDescriptiveSuffix"), "Item"));

    for (const FolderNameControl& folder : FOLDER_NAME_CONTROLS)
    {
        getChild<LLLineEditor>(folder.control)->setText(
            nonEmpty(gSavedPerAccountSettings.getString(folder.setting), folder.fallback));
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

    gSavedPerAccountSettings.setBOOL(
        "FSInventoryAssistantAutoBodyGender",
        getChild<LLCheckBoxCtrl>("auto_body_gender")->getValue().asBoolean());

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

    gSavedPerAccountSettings.setString(
        "FSInventoryAssistantFolderNamingMode",
        getChild<LLRadioGroup>("folder_naming_mode")->getValue().asString());
    gSavedPerAccountSettings.setString(
        "FSInventoryAssistantDescriptiveSuffix",
        nonEmpty(getChild<LLLineEditor>("descriptive_suffix")->getText(), "Item"));

    for (const FolderNameControl& folder : FOLDER_NAME_CONTROLS)
    {
        gSavedPerAccountSettings.setString(
            folder.setting,
            nonEmpty(getChild<LLLineEditor>(folder.control)->getText(), folder.fallback));
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
