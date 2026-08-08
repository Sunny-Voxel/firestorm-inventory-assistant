#include "llviewerprecompiledheaders.h"

#include "llfloaterinventoryassistant.h"

#include "llbutton.h"
#include "llfloaterreg.h"
#include "llscrolllistctrl.h"
#include "llscrolllistitem.h"
#include "lltextbox.h"
#include "lltexteditor.h"

#include "fsinventorytools/fsinventoryscan.h"
#include "fsinventorytools/ctsdestinationplanner.h"
#include "fsinventorytools/ctsexecutionplan.h"
#include "fsinventorytools/ctsexecutor.h"
#include "fsinventorytools/ctsundomanager.h"

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

const char* statusText(bool correct, bool folderMissing, bool parkedInAddOn)
{
    if (parkedInAddOn) return "ADD ON";
    if (correct) return "OK";
    if (folderMissing) return "NEW FOLDER";
    return "MOVE";
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
        boost::bind(&LLFloaterInventoryAssistant::onRefreshClicked, this));
    getChild<LLButton>("repair_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onRepairClicked, this));
    getChild<LLButton>("organize_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onOrganizeClicked, this));
    getChild<LLButton>("undo_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onUndoClicked, this));
    getChild<LLButton>("bodies_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onBodiesClicked, this));
    getChild<LLButton>("close_btn")->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onCloseClicked, this));

    mPlanList->setCommitCallback(
        boost::bind(&LLFloaterInventoryAssistant::onPlanSelectionChanged, this));

    resetWorkflowButtons();
    return true;
}

void LLFloaterInventoryAssistant::onOpen(const LLSD& key)
{
    if (key.isMap() && key.has("folder_id"))
        mFolderId = key["folder_id"].asUUID();

    analyzeFolder();
}

void LLFloaterInventoryAssistant::resetWorkflowButtons()
{
    mPendingAction = PendingAction::None;
    getChild<LLButton>("repair_btn")->setLabel("Repair");
    getChild<LLButton>("organize_btn")->setLabel("Organize");
    getChild<LLButton>("repair_btn")->setEnabled(true);
    getChild<LLButton>("organize_btn")->setEnabled(true);
    getChild<LLButton>("undo_btn")->setEnabled(
        CTSUndoManager::instance().canUndo());
}

void LLFloaterInventoryAssistant::onRefreshClicked()
{
    analyzeFolder();
}

void LLFloaterInventoryAssistant::onRepairClicked()
{
    if (mPendingAction == PendingAction::Repair)
    {
        executePendingAction();
        return;
    }
    prepareAction(PendingAction::Repair);
}

void LLFloaterInventoryAssistant::onOrganizeClicked()
{
    if (mPendingAction == PendingAction::Organize)
    {
        executePendingAction();
        return;
    }
    prepareAction(PendingAction::Organize);
}

void LLFloaterInventoryAssistant::onBodiesClicked()
{
    LLFloaterReg::showInstance("inventory_assistant_settings");
}

void LLFloaterInventoryAssistant::onCloseClicked()
{
    closeFloater();
}

void LLFloaterInventoryAssistant::analyzeFolder()
{
    resetWorkflowButtons();

    if (!mPlanList || !mDetails || mFolderId.isNull())
        return;

    FSInventoryScanResult scan =
        FSInventoryScanner::scanProductFolder(mFolderId);

    CTSDestinationPlan plan =
        CTSDestinationPlanner::build(scan);

    mProductText->setText(LLStringExplicit(scan.productName));
    mPlanList->clearRows();

    std::size_t moveCount = 0;
    std::size_t correctCount = 0;
    std::size_t parkedCount = 0;

    for (const auto& group : plan)
    {
        const std::string& destination = group.first;
        const bool destinationExists =
            folderExists(scan.existingFolderNames, destination);

        for (const CTSDestinationPlannedItem& planned : group.second)
        {
            const FSInventoryScannedItem& item = planned.item;
            const bool alreadyCorrect =
                LLStringUtil::compareInsensitive(
                    item.sourceFolderName, destination) == 0;

            if (planned.parkedInAddOn) ++parkedCount;
            if (alreadyCorrect) ++correctCount;
            else ++moveCount;

            LLSD row;
            row["value"] = item.itemId;
            LLSD& columns = row["columns"];

            columns[0]["column"] = "status";
            columns[0]["value"] =
                statusText(alreadyCorrect, !destinationExists, planned.parkedInAddOn);

            columns[1]["column"] = "destination";
            columns[1]["value"] = destination;

            columns[2]["column"] = "item";
            columns[2]["value"] = item.itemName;

            columns[3]["column"] = "source";
            columns[3]["value"] =
                item.sourceFolderId == scan.productFolderId
                    ? "Product folder"
                    : item.sourceFolderName;

            mPlanList->addElement(row);
        }
    }

    mItemsText->setText(LLStringExplicit(std::to_string(scan.items.size())));
    mCorrectText->setText(LLStringExplicit(std::to_string(correctCount)));
    mMovesText->setText(LLStringExplicit(std::to_string(moveCount)));
    mFoldersText->setText(LLStringExplicit(std::to_string(plan.size())));

    std::ostringstream status;
    if (moveCount == 0)
    {
        status << "Ready - no changes needed";
    }
    else
    {
        status << moveCount << " changes found";
        if (parkedCount)
            status << " | " << parkedCount << " items kept in Add On";
    }

    mStatusText->setText(LLStringExplicit(status.str()));
    getChild<LLButton>("repair_btn")->setEnabled(moveCount > 0);
    getChild<LLButton>("organize_btn")->setEnabled(moveCount > 0);

    mDetails->setText(LLStringExplicit(
        "Analysis runs automatically when the window opens.\n\n"
        "OK = already correct\n"
        "MOVE = move to existing folder\n"
        "NEW FOLDER = create destination folder\n"
        "ADD ON = body is disabled or confidence is low\n\n"
        "Repair/Organize: first click shows the plan.\n"
        "The same button then changes to Execute."));
}

void LLFloaterInventoryAssistant::prepareAction(PendingAction action)
{
    if (mFolderId.isNull())
        return;

    FSInventoryScanResult scan =
        FSInventoryScanner::scanProductFolder(mFolderId);
    CTSDestinationPlan destinationPlan =
        CTSDestinationPlanner::build(scan);

    const CTSExecutionMode mode =
        action == PendingAction::Repair
            ? CTSExecutionMode::Repair
            : CTSExecutionMode::Organize;

    CTSExecutionPlan executionPlan =
        CTSExecutionPlanBuilder::build(scan, destinationPlan, mode);

    if (executionPlan.empty())
    {
        mStatusText->setText(LLStringExplicit("No changes are needed."));
        return;
    }

    mPendingAction = action;

    if (action == PendingAction::Repair)
    {
        getChild<LLButton>("repair_btn")->setLabel("Execute Repair");
        getChild<LLButton>("organize_btn")->setEnabled(false);
    }
    else
    {
        getChild<LLButton>("organize_btn")->setLabel("Execute Organize");
        getChild<LLButton>("repair_btn")->setEnabled(false);
    }

    std::ostringstream details;
    details << (action == PendingAction::Repair ? "REPAIR" : "ORGANIZE")
            << " - REVIEW PLAN\n\n"
            << "New folders: " << executionPlan.foldersToCreate.size() << "\n"
            << "Moves: " << executionPlan.moves.size() << "\n\n";

    for (const CTSPlannedMove& move : executionPlan.moves)
    {
        details << move.itemName << "\n  "
                << move.sourceFolderName << " -> "
                << move.destinationFolderName << "\n\n";
    }

    details << "Click the same button again to execute.";

    mDetails->setText(LLStringExplicit(details.str()));
    mStatusText->setText(LLStringExplicit("Plan ready - review and execute"));
}

void LLFloaterInventoryAssistant::executePendingAction()
{
    if (mPendingAction == PendingAction::None || mFolderId.isNull())
        return;

    const PendingAction action = mPendingAction;

    FSInventoryScanResult scan =
        FSInventoryScanner::scanProductFolder(mFolderId);
    CTSDestinationPlan destinationPlan =
        CTSDestinationPlanner::build(scan);
    CTSExecutionPlan executionPlan =
        CTSExecutionPlanBuilder::build(
            scan,
            destinationPlan,
            action == PendingAction::Repair
                ? CTSExecutionMode::Repair
                : CTSExecutionMode::Organize);

    mPendingAction = PendingAction::None;
    getChild<LLButton>("repair_btn")->setEnabled(false);
    getChild<LLButton>("organize_btn")->setEnabled(false);
    mStatusText->setText(LLStringExplicit("Executing changes..."));

    LLHandle<LLFloater> handle = getHandle();

    CTSExecutor::execute(
        mFolderId,
        scan,
        executionPlan,
        [handle](const CTSExecutionResult& result)
        {
            LLFloater* floater = handle.get();
            auto* self =
                dynamic_cast<LLFloaterInventoryAssistant*>(floater);
            if (self)
                self->onExecutionComplete(result);
        });
}

void LLFloaterInventoryAssistant::onExecutionComplete(
    const CTSExecutionResult& result)
{
    analyzeFolder();
    mStatusText->setText(LLStringExplicit(result.message));

    std::ostringstream details;
    details << (result.success ? "DONE" : "STOPPED")
            << "\n\n" << result.message
            << "\n\nFlyttet: " << result.itemsMoved
            << "\nMapper oprettet: " << result.foldersCreated;

    if (CTSUndoManager::instance().canUndo())
        details << "\n\nUndo er tilgængelig.";

    mDetails->setText(LLStringExplicit(details.str()));
    getChild<LLButton>("undo_btn")->setEnabled(
        CTSUndoManager::instance().canUndo());
}

void LLFloaterInventoryAssistant::onUndoClicked()
{
    if (!CTSUndoManager::instance().canUndo())
    {
        mStatusText->setText(LLStringExplicit("Undo is not available."));
        return;
    }

    getChild<LLButton>("undo_btn")->setEnabled(false);
    mStatusText->setText(LLStringExplicit("Undoing last change..."));

    LLHandle<LLFloater> handle = getHandle();

    CTSUndoManager::instance().undo(
        [handle](const CTSUndoResult& result)
        {
            LLFloater* floater = handle.get();
            auto* self =
                dynamic_cast<LLFloaterInventoryAssistant*>(floater);
            if (self)
                self->onUndoComplete(result);
        });
}

void LLFloaterInventoryAssistant::onUndoComplete(
    const CTSUndoResult& result)
{
    analyzeFolder();
    mStatusText->setText(LLStringExplicit(result.message));
    mDetails->setText(LLStringExplicit(result.message));
}

void LLFloaterInventoryAssistant::onPlanSelectionChanged()
{
    if (!mPlanList || !mDetails)
        return;

    LLScrollListItem* selected = mPlanList->getFirstSelected();
    if (!selected)
        return;

    std::ostringstream details;
    details << "Selected item\n------------------------------\n\n";

    if (selected->getColumn(2))
        details << "Item:\n"
                << selected->getColumn(2)->getValue().asString()
                << "\n\n";
    if (selected->getColumn(0))
        details << "Status: "
                << selected->getColumn(0)->getValue().asString()
                << "\n";
    if (selected->getColumn(3))
        details << "From: "
                << selected->getColumn(3)->getValue().asString()
                << "\n";
    if (selected->getColumn(1))
        details << "To: "
                << selected->getColumn(1)->getValue().asString()
                << "\n";

    mDetails->setText(LLStringExplicit(details.str()));
}
