#include "../llviewerprecompiledheaders.h"

#include "ctsexecutor.h"

#include "../llinventorymodel.h"
#include "../llviewerinventory.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <set>

namespace
{
std::string lowerCopy(std::string value)
{
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });
    return value;
}
}

struct CTSExecutor::Context
{
    LLUUID productFolderId;
    FSInventoryScanResult scan;
    CTSExecutionPlan plan;
    completion_func_t completion;

    std::map<std::string, LLUUID> destinationFolders;
    std::size_t nextFolderIndex = 0;

    CTSUndoTransaction transaction;
    CTSExecutionResult result;
};

bool CTSExecutor::preflight(
    const LLUUID& productFolderId,
    const FSInventoryScanResult& scan,
    const CTSExecutionPlan& plan,
    std::map<std::string, LLUUID>& destinationFolders,
    std::string& error)
{
    if (productFolderId.isNull() || !gInventory.getCategory(productFolderId))
    {
        error = "Produktmappen findes ikke længere.";
        return false;
    }

    LLInventoryModel::cat_array_t* categories = nullptr;
    LLInventoryModel::item_array_t* rootItems = nullptr;
    gInventory.getDirectDescendentsOf(productFolderId, categories, rootItems);

    if (categories)
    {
        for (const auto& category : *categories)
        {
            if (!category) continue;
            destinationFolders[lowerCopy(category->getName())] = category->getUUID();
        }
    }

    for (const CTSPlannedMove& move : plan.moves)
    {
        LLViewerInventoryItem* item = gInventory.getItem(move.itemId);
        if (!item)
        {
            error = "Et planlagt item findes ikke længere: " + move.itemName;
            return false;
        }

        if (item->getParentUUID() != move.sourceFolderId)
        {
            error = "Inventory blev ændret efter preview. Kør Analyze igen: " + move.itemName;
            return false;
        }

        const std::string key = lowerCopy(move.destinationFolderName);
        if (destinationFolders.find(key) != destinationFolders.end())
            continue;

        bool plannedCreate = false;
        for (const CTSPlannedFolderCreate& folder : plan.foldersToCreate)
        {
            if (lowerCopy(folder.folderName) == key)
            {
                plannedCreate = true;
                break;
            }
        }

        if (!plannedCreate)
        {
            error = "Destinationen kan ikke findes eller oprettes: " + move.destinationFolderName;
            return false;
        }
    }

    return true;
}

void CTSExecutor::execute(
    const LLUUID& productFolderId,
    const FSInventoryScanResult& scan,
    const CTSExecutionPlan& plan,
    completion_func_t completion)
{
    auto context = std::make_shared<Context>();
    context->productFolderId = productFolderId;
    context->scan = scan;
    context->plan = plan;
    context->completion = completion;
    context->transaction.productFolderId = productFolderId;
    context->transaction.productName = scan.productName;

    std::string error;
    if (!preflight(
            productFolderId,
            scan,
            plan,
            context->destinationFolders,
            error))
    {
        finish(context, false, error);
        return;
    }

    if (plan.moves.empty())
    {
        finish(context, true, "Ingen flytninger er nødvendige.");
        return;
    }

    createNextFolder(context);
}

void CTSExecutor::createNextFolder(const std::shared_ptr<Context>& context)
{
    while (context->nextFolderIndex < context->plan.foldersToCreate.size())
    {
        const CTSPlannedFolderCreate& folder =
            context->plan.foldersToCreate[context->nextFolderIndex];

        const std::string key = lowerCopy(folder.folderName);

        if (context->destinationFolders.find(key) != context->destinationFolders.end())
        {
            ++context->nextFolderIndex;
            continue;
        }

        const std::string folderName = folder.folderName;

        gInventory.createNewCategory(
            context->productFolderId,
            LLFolderType::FT_NONE,
            folderName,
            [context, folderName](const LLUUID& newFolderId)
            {
                if (newFolderId.isNull())
                {
                    finish(context, false, "Kunne ikke oprette mappen: " + folderName);
                    return;
                }

                context->destinationFolders[lowerCopy(folderName)] = newFolderId;
                context->transaction.createdFolderIds.push_back(newFolderId);
                ++context->result.foldersCreated;
                ++context->nextFolderIndex;
                createNextFolder(context);
            });

        return;
    }

    moveItems(context);
}

void CTSExecutor::moveItems(const std::shared_ptr<Context>& context)
{
    for (const CTSPlannedMove& move : context->plan.moves)
    {
        LLViewerInventoryItem* item = gInventory.getItem(move.itemId);

        if (!item || item->getParentUUID() != move.sourceFolderId)
        {
            rollbackMovedItems(context);
            finish(context, false, "Item blev ændret under udførelsen: " + move.itemName);
            return;
        }

        const auto destinationIt =
            context->destinationFolders.find(lowerCopy(move.destinationFolderName));

        if (destinationIt == context->destinationFolders.end())
        {
            rollbackMovedItems(context);
            finish(context, false, "Destination mangler: " + move.destinationFolderName);
            return;
        }

        const LLUUID originalParent = item->getParentUUID();
        const LLUUID newParent = destinationIt->second;

        gInventory.changeItemParent(item, newParent, false);

        LLViewerInventoryItem* updated = gInventory.getItem(move.itemId);
        if (!updated || updated->getParentUUID() != newParent)
        {
            rollbackMovedItems(context);
            finish(context, false, "Flytningen kunne ikke bekræftes: " + move.itemName);
            return;
        }

        CTSUndoMove undoMove;
        undoMove.itemId = move.itemId;
        undoMove.originalParentId = originalParent;
        undoMove.newParentId = newParent;
        undoMove.itemName = move.itemName;

        context->transaction.moves.push_back(undoMove);
        ++context->result.itemsMoved;
    }

    // Ryd op i eksisterende source-foldere som denne transaction har tømt.
    // Kun direkte undermapper til produktet røres.
    std::set<LLUUID> sourceFolderIds;

    for (const CTSUndoMove& move : context->transaction.moves)
    {
        if (move.originalParentId != context->productFolderId)
            sourceFolderIds.insert(move.originalParentId);
    }

    for (const LLUUID& sourceFolderId : sourceFolderIds)
    {
        if (std::find(
                context->transaction.createdFolderIds.begin(),
                context->transaction.createdFolderIds.end(),
                sourceFolderId)
            != context->transaction.createdFolderIds.end())
        {
            continue;
        }

        LLViewerInventoryCategory* sourceFolder =
            gInventory.getCategory(sourceFolderId);

        if (!sourceFolder)
            continue;

        if (sourceFolder->getParentUUID() != context->productFolderId)
            continue;

        LLInventoryModel::cat_array_t* childCategories = nullptr;
        LLInventoryModel::item_array_t* childItems = nullptr;

        gInventory.getDirectDescendentsOf(
            sourceFolderId,
            childCategories,
            childItems);

        const bool hasCategories =
            childCategories && !childCategories->empty();

        const bool hasItems =
            childItems && !childItems->empty();

        if (hasCategories || hasItems)
            continue;

        CTSRemovedSourceFolder removed;
        removed.folderId = sourceFolderId;
        removed.originalParentId = sourceFolder->getParentUUID();
        removed.folderName = sourceFolder->getName();

        context->transaction.removedSourceFolders.push_back(removed);

        LL_INFOS("FSInventoryTools")
            << "Moving empty source folder to Trash after execute: "
            << removed.folderName
            << " id="
            << removed.folderId
            << LL_ENDL;

        gInventory.removeCategory(sourceFolderId);
    }

    CTSUndoManager::instance().setLastTransaction(context->transaction);

    std::ostringstream message;
    message << context->result.itemsMoved << " items flyttet";
    if (context->result.foldersCreated)
        message << ", " << context->result.foldersCreated << " mapper oprettet";
    message << ".";

    finish(context, true, message.str());
}

void CTSExecutor::rollbackMovedItems(const std::shared_ptr<Context>& context)
{
    for (auto it = context->transaction.moves.rbegin();
         it != context->transaction.moves.rend();
         ++it)
    {
        LLViewerInventoryItem* item = gInventory.getItem(it->itemId);
        if (item && item->getParentUUID() == it->newParentId)
            gInventory.changeItemParent(item, it->originalParentId, false);
    }

    context->transaction.moves.clear();
    context->result.itemsMoved = 0;
}

void CTSExecutor::finish(
    const std::shared_ptr<Context>& context,
    bool success,
    const std::string& message)
{
    context->result.success = success;
    context->result.message = message;

    LL_INFOS("FSInventoryTools")
        << "Execute result: success=" << success
        << " | moved=" << context->result.itemsMoved
        << " | created=" << context->result.foldersCreated
        << " | " << message << LL_ENDL;

    if (context->completion)
        context->completion(context->result);
}
