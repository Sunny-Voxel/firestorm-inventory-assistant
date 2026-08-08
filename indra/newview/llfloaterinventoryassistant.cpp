#include "llviewerprecompiledheaders.h"

#include "llfloaterinventoryassistant.h"

#include "llbutton.h"
#include "llscrolllistctrl.h"
#include "llscrolllistitem.h"
#include "lltextbox.h"
#include "lltexteditor.h"

#include "fsinventorytools/fsinventoryscan.h"
#include "fsinventorytools/ctsdestinationplanner.h"
#include "fsinventorytools/ctsexecutionplan.h"

#include <sstream>
#include <string>

namespace
{
bool folderExists(const std::vector<std::string>& folders, const std::string& wanted)
{
    for (const std::string& folder : folders)
    {
        if (LLStringUtil::compareInsensitive(folder, wanted) == 0)
            return true;
    }
    return false;
}

const char* statusText(bool correct, bool folderMissing)
{
    if (correct) return "OK";
    if (folderMissing) return "NY MAPPE";
    return "FLYT";
}
}

LLFloaterInventoryAssistant::LLFloaterInventoryAssistant(const LLSD& key)
: LLFloater(key)
{
}

bool LLFloaterInventoryAssistant::postBuild()
{
    mPlanList = getChild<LLScrollListCtrl>("plan_list");
    mDetails = getChild<LLTextEditor>("details_text");
    mProductText = getChild<LLTextBox>("product_text");
    mStatusText = getChild<LLTextBox>("status_text");
    mItemsText = getChild<LLTextBox>("items_value");
    mCorrectText = getChild<LLTextBox>("correct_value");
    mMovesText = getChild<LLTextBox>("moves_value");
    mFoldersText = getChild<LLTextBox>("folders_value");

    getChild<LLButton>("analyze_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onAnalyzeClicked, this));
    getChild<LLButton>("repair_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onRepairClicked, this));
    getChild<LLButton>("organize_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onOrganizeClicked, this));
    getChild<LLButton>("close_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onCloseClicked, this));

    getChild<LLButton>("undo_btn")->setEnabled(false);
    mPlanList->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onPlanSelectionChanged, this));

    return true;
}

void LLFloaterInventoryAssistant::onOpen(const LLSD& key)
{
    if (key.isMap() && key.has("folder_id"))
        mFolderId = key["folder_id"].asUUID();
    analyzeFolder();
}

void LLFloaterInventoryAssistant::onAnalyzeClicked() { analyzeFolder(); }
void LLFloaterInventoryAssistant::onRepairClicked() { showDryRun(true); }
void LLFloaterInventoryAssistant::onOrganizeClicked() { showDryRun(false); }
void LLFloaterInventoryAssistant::onCloseClicked() { closeFloater(); }

void LLFloaterInventoryAssistant::analyzeFolder()
{
    if (!mPlanList || !mDetails || mFolderId.isNull())
        return;

    FSInventoryScanResult scan = FSInventoryScanner::scanProductFolder(mFolderId);
    CTSDestinationPlan plan = CTSDestinationPlanner::build(scan);

    mProductText->setText(LLStringExplicit(scan.productName));
    mPlanList->clearRows();

    std::size_t moveCount = 0;
    std::size_t correctCount = 0;

    for (const auto& group : plan)
    {
        const std::string& destination = group.first;
        const bool destinationExists = folderExists(scan.existingFolderNames, destination);

        for (const CTSDestinationPlannedItem& planned : group.second)
        {
            const FSInventoryScannedItem& item = planned.item;
            const bool alreadyCorrect =
                LLStringUtil::compareInsensitive(item.sourceFolderName, destination) == 0;

            alreadyCorrect ? ++correctCount : ++moveCount;

            LLSD row;
            row["value"] = item.itemId;
            LLSD& columns = row["columns"];
            columns[0]["column"] = "status";
            columns[0]["value"] = statusText(alreadyCorrect, !destinationExists);
            columns[1]["column"] = "destination";
            columns[1]["value"] = destination;
            columns[2]["column"] = "item";
            columns[2]["value"] = item.itemName;
            columns[3]["column"] = "source";
            columns[3]["value"] =
                item.sourceFolderId == scan.productFolderId ? "Produktmappe" : item.sourceFolderName;

            mPlanList->addElement(row);
        }
    }

    mItemsText->setText(LLStringExplicit(std::to_string(scan.items.size())));
    mCorrectText->setText(LLStringExplicit(std::to_string(correctCount)));
    mMovesText->setText(LLStringExplicit(std::to_string(moveCount)));
    mFoldersText->setText(LLStringExplicit(std::to_string(plan.size())));

    if (moveCount == 0)
        mStatusText->setText(LLStringExplicit("Klar - ingen ændringer nødvendige"));
    else if (moveCount <= 3)
        mStatusText->setText(LLStringExplicit("Næsten klar - få ændringer foreslået"));
    else
        mStatusText->setText(LLStringExplicit("Ændringer fundet - gennemgå planen"));

    getChild<LLButton>("repair_btn")->setEnabled(moveCount > 0);
    getChild<LLButton>("organize_btn")->setEnabled(moveCount > 0);

    std::ostringstream details;
    details << "Vælg en linje i listen for detaljer.\n\n"
            << "OK = item ligger korrekt\n"
            << "FLYT = item flyttes til eksisterende mappe\n"
            << "NY MAPPE = destinationsmappen mangler\n\n"
            << "Repair og Organize er stadig dry-run.";
    mDetails->setText(LLStringExplicit(details.str()));
}

void LLFloaterInventoryAssistant::showDryRun(bool repairMode)
{
    if (!mDetails || mFolderId.isNull())
        return;

    FSInventoryScanResult scan = FSInventoryScanner::scanProductFolder(mFolderId);
    CTSDestinationPlan destinationPlan = CTSDestinationPlanner::build(scan);
    CTSExecutionPlan executionPlan = CTSExecutionPlanBuilder::build(
        scan, destinationPlan,
        repairMode ? CTSExecutionMode::Repair : CTSExecutionMode::Organize);

    std::ostringstream details;
    details << (repairMode ? "REPAIR" : "ORGANIZE") << " - DRY RUN\n\n"
            << "Nye mapper: " << executionPlan.foldersToCreate.size() << "\n";

    for (const CTSPlannedFolderCreate& folder : executionPlan.foldersToCreate)
        details << "  + " << folder.folderName << "\n";

    details << "\nFlytninger: " << executionPlan.moves.size() << "\n";

    for (const CTSPlannedMove& move : executionPlan.moves)
    {
        details << "\n" << move.itemName << "\n  "
                << (move.sourceFolderId == scan.productFolderId
                    ? "Produktmappe" : move.sourceFolderName)
                << " -> " << move.destinationFolderName << "\n";
    }

    details << "\nIngen ændringer er udført.";
    mDetails->setText(LLStringExplicit(details.str()));
}

void LLFloaterInventoryAssistant::onPlanSelectionChanged()
{
    if (!mPlanList || !mDetails)
        return;

    LLScrollListItem* selected = mPlanList->getFirstSelected();
    if (!selected)
        return;

    std::ostringstream details;
    details << "Valgt item\n------------------------------\n\n";

    if (selected->getColumn(2))
        details << "Item:\n" << selected->getColumn(2)->getValue().asString() << "\n\n";
    if (selected->getColumn(0))
        details << "Status: " << selected->getColumn(0)->getValue().asString() << "\n";
    if (selected->getColumn(3))
        details << "Fra: " << selected->getColumn(3)->getValue().asString() << "\n";
    if (selected->getColumn(1))
        details << "Til: " << selected->getColumn(1)->getValue().asString() << "\n";

    mDetails->setText(LLStringExplicit(details.str()));
}
