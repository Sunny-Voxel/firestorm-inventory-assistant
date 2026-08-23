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
#include "fsinventorytools/ctssplitexecutor.h"
#include "fsinventorytools/ctsbodysettings.h"
#include "fsinventorytools/ctsrules.h"
#include "fsinventorytools/ctsundomanager.h"

#include <sstream>
#include <string>
#include <set>

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

    // CTS_DUPLICATE_PREVIEW_0019B
    // Duplicate cleanup is planned in CTSExecutionPlanBuilder.
    // Build a dry-run plan here so duplicates are visible in the
    // main preview before Repair/Organize is pressed.
    CTSExecutionPlan previewExecutionPlan =
        CTSExecutionPlanBuilder::build(
            scan,
            plan,
            CTSExecutionMode::Repair);

    std::set<LLUUID> previewDuplicateIds;
    for (const CTSPlannedDuplicateRemoval& duplicate :
         previewExecutionPlan.duplicatesToRemove)
    {
        previewDuplicateIds.insert(duplicate.itemId);
    }

    mProductText->setText(LLStringExplicit(scan.productName));
    mPlanList->clearRows();

    const bool splitMode =
        CTSBodySettings::splitMainFolders();
    const std::vector<std::string> splitBodies =
        CTSSplitExecutor::applicableTargets(scan);

    std::set<std::string> splitTargetSet(
        splitBodies.begin(), splitBodies.end());

    std::size_t moveCount = 0;
    std::size_t correctCount = 0;
    std::size_t parkedCount = 0;
    std::size_t duplicateCount = 0;

    if (splitMode)
    {
        // 0021e: show the actual split result instead of the legacy plan.
        for (const FSInventoryScannedItem& item : scan.items)
        {
            const CTSItemClassification classification =
                CTSItemClassifier::classify(item.itemName);
            const std::string special =
                CTSRules::classifyItem(item.itemName);

            std::string status;
            std::string destination;

            const std::string target =
                !classification.body.empty()
                    ? classification.body
                    : classification.head;

            if (special == "HUD")
            {
                status = "SHARED";
                destination =
                    CTSBodySettings::splitHUDFolder()
                        ? "ALL PACKAGES / " +
                          CTSBodySettings::hudFolderName()
                        : "ALL PACKAGES / body folder";
            }
            else if (special == "Alpha")
            {
                status = "SHARED";
                destination =
                    CTSBodySettings::splitAlphaFolder()
                        ? "ALL PACKAGES / " +
                          CTSBodySettings::alphaFolderName()
                        : "ALL PACKAGES / body folder";
            }
            else if (!target.empty())
            {
                if (splitTargetSet.count(target))
                {
                    status = "PACKAGE";
                    destination =
                        scan.productName + " - " +
                        target + " / " + target;
                }
                else
                {
                    status = "SKIP";
                    destination =
                        target + " (not selected)";
                }
            }
            else
            {
                status = "SHARED";
                destination =
                    "ALL PACKAGES / " +
                    CTSBodySettings::addOnFolderName();
            }

            LLSD row;
            row["value"] = item.itemId;
            LLSD& columns = row["columns"];

            columns[0]["column"] = "status";
            columns[0]["value"] = status;

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
    else
    {
    for (const auto& group : plan)
    {
        const std::string& destination = group.first;
        const bool destinationExists =
            folderExists(scan.existingFolderNames, destination);

        for (const CTSDestinationPlannedItem& planned : group.second)
        {
            const FSInventoryScannedItem& item = planned.item;
            const bool duplicate =
                previewDuplicateIds.count(item.itemId) != 0;

            const bool alreadyCorrect =
                !duplicate &&
                LLStringUtil::compareInsensitive(
                    item.sourceFolderName, destination) == 0;

            if (planned.parkedInAddOn) ++parkedCount;

            if (duplicate)
                ++duplicateCount;
            else if (alreadyCorrect)
                ++correctCount;
            else
                ++moveCount;

            LLSD row;
            row["value"] = item.itemId;
            LLSD& columns = row["columns"];

            columns[0]["column"] = "status";
            columns[0]["value"] =
                duplicate
                    ? "DUPLICATE"
                    : statusText(
                        alreadyCorrect,
                        !destinationExists,
                        planned.parkedInAddOn);

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

    }

    mItemsText->setText(LLStringExplicit(std::to_string(scan.items.size())));
    mCorrectText->setText(LLStringExplicit(std::to_string(correctCount)));
    mMovesText->setText(LLStringExplicit(std::to_string(moveCount)));
    mFoldersText->setText(LLStringExplicit(std::to_string(plan.size())));

    // CTS_SPLIT_PREVIEW_0021B
    const std::size_t normalActionCount =
        moveCount + duplicateCount;
    const std::size_t actionCount =
        splitMode && !splitBodies.empty()
            ? std::max<std::size_t>(normalActionCount, 1)
            : normalActionCount;

    std::ostringstream status;
    if (splitMode && !splitBodies.empty())
    {
        status << "Per-variant package mode | "
               << splitBodies.size()
               << " package"
               << (splitBodies.size() == 1 ? "" : "s")
               << " will be created";
    }
    else if (actionCount == 0)
    {
        status << "Ready - no changes needed";
    }
    else
    {
        status << actionCount << " changes found";

        if (duplicateCount)
            status << " | " << duplicateCount << " duplicates";

        if (parkedCount)
            status << " | " << parkedCount << " items kept in Add On";
    }

    mStatusText->setText(LLStringExplicit(status.str()));

    const bool canExecute =
        actionCount > 0 &&
        (!splitMode || !splitBodies.empty());

    getChild<LLButton>("repair_btn")->setEnabled(
        canExecute);
    getChild<LLButton>("organize_btn")->setEnabled(
        canExecute);

    if (splitMode)
    {
        std::size_t bodyItemCopies = 0;
        std::size_t sharedItemCopies = 0;

        for (const FSInventoryScannedItem& scanned : scan.items)
        {
            const CTSItemClassification classification =
                CTSItemClassifier::classify(scanned.itemName);
            const std::string special =
                CTSRules::classifyItem(scanned.itemName);

            if (special == "HUD" || special == "Alpha" ||
                (classification.body.empty() &&
                 classification.head.empty()))
            {
                sharedItemCopies += splitBodies.size();
            }
            else
            {
                const std::string target =
                    !classification.body.empty()
                        ? classification.body
                        : classification.head;
                for (const std::string& body : splitBodies)
                {
                    if (target == body)
                    {
                        ++bodyItemCopies;
                        break;
                    }
                }
            }
        }

        const std::size_t folderCountPerBody =
            2 +
            (CTSBodySettings::splitHUDFolder() ? 1 : 0) +
            (CTSBodySettings::splitAlphaFolder() ? 1 : 0);

        std::ostringstream splitDetails;
        splitDetails
            << "PER-VARIANT PACKAGE MODE\n\n";

        if (splitBodies.empty())
        {
            splitDetails
                << "No selected body or head was detected in this product.\n"
                << "Execute is disabled for split mode.";
        }
        else
        {
            splitDetails
                << "Original product: unchanged\n"
                << "Variants found: " << splitBodies.size() << "\n"
                << "Item copies: "
                << (bodyItemCopies + sharedItemCopies) << "\n"
                << "New folders: "
                << (folderCountPerBody * splitBodies.size()) << "\n\n"
                << "MAIN FOLDERS\n";

            for (const std::string& body : splitBodies)
            {
                splitDetails
                    << "  " << scan.productName
                    << " - " << body << "\n";
            }

            splitDetails
                << "\nHUD: "
                << (CTSBodySettings::splitHUDFolder()
                        ? CTSBodySettings::hudFolderName()
                        : "copy directly into each body folder")
                << "\nAlpha: "
                << (CTSBodySettings::splitAlphaFolder()
                        ? CTSBodySettings::alphaFolderName()
                        : "copy directly into each body folder")
                << "\nAdd On: "
                << CTSBodySettings::addOnFolderName()
                << " in every package\n\n"
                << "Only selected bodies actually found in the product "
                   "get a package.";
        }

        mDetails->setText(LLStringExplicit(splitDetails.str()));
    }
    else
    {
        mDetails->setText(LLStringExplicit(
            "Analysis runs automatically when the window opens.\n\n"
            "OK = already correct\n"
            "DUPLICATE = extra item with the same visible name\n"
            "MOVE = move to existing folder\n"
            "NEW FOLDER = create destination folder\n"
            "ADD ON = body is disabled or confidence is low\n\n"
            "Repair/Organize: first click shows the plan.\n"
            "The same button then changes to Execute."));
    }
}

void LLFloaterInventoryAssistant::prepareAction(PendingAction action)
{
    if (mFolderId.isNull())
        return;

    // CTS_SPLIT_REVIEW_0021B
    if (CTSBodySettings::splitMainFolders())
    {
        FSInventoryScanResult splitScan =
            FSInventoryScanner::scanProductFolder(mFolderId);

        const std::vector<std::string> bodies =
            CTSSplitExecutor::applicableTargets(splitScan);

        if (bodies.empty())
        {
            mStatusText->setText(
                LLStringExplicit(
                    "No selected body or head was detected in this product."));
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
        details
            << "PER-VARIANT PACKAGE MODE - REVIEW PLAN\n\n"
            << "Original product: unchanged\n"
            << "New main folders: " << bodies.size() << "\n\n";

        for (const std::string& body : bodies)
            details << "CREATE  "
                    << splitScan.productName
                    << " - " << body << "\n";

        details
            << "\nHUD: "
            << (CTSBodySettings::splitHUDFolder()
                    ? CTSBodySettings::hudFolderName()
                    : "copy directly into each body folder")
            << "\nAlpha: "
            << (CTSBodySettings::splitAlphaFolder()
                    ? CTSBodySettings::alphaFolderName()
                    : "copy directly into each body folder")
            << "\nAdd On: "
            << CTSBodySettings::addOnFolderName()
            << "\n\nClick the same button again to execute.";

        mDetails->setText(LLStringExplicit(details.str()));
        mStatusText->setText(
            LLStringExplicit("Split package plan ready - review and execute"));
        return;
    }

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

    for (const CTSPlannedFolderRename& rename :
         executionPlan.foldersToRename)
    {
        details
            << "RENAME  "
            << rename.oldName
            << " -> "
            << rename.newName
            << "\n";
    }

    if (!executionPlan.foldersToRename.empty())
        details << "\n";

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

    // CTS_SPLIT_EXECUTE_0021B
    if (CTSBodySettings::splitMainFolders())
    {
        CTSSplitExecutor::execute(
            mFolderId,
            scan,
            [handle](const CTSExecutionResult& result)
            {
                LLFloater* floater = handle.get();
                auto* self =
                    dynamic_cast<LLFloaterInventoryAssistant*>(floater);
                if (self)
                    self->onExecutionComplete(result);
            });
        return;
    }

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
            << "\n\nMoved: " << result.itemsMoved
            << "\nCopied: " << result.itemsCopied
            << "\nFolders created: " << result.foldersCreated;

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
