#include "../llviewerprecompiledheaders.h"
#include "ctsundomanager.h"
#include "../llinventorymodel.h"
#include "../llviewerinventory.h"
#include <sstream>

CTSUndoManager& CTSUndoManager::instance()
{
    static CTSUndoManager instance;
    return instance;
}

void CTSUndoManager::setLastTransaction(const CTSUndoTransaction& transaction)
{
    mLastTransaction = transaction;
    mHasTransaction =
        !transaction.moves.empty() ||
        !transaction.removedSourceFolders.empty();

    LL_INFOS("FSInventoryTools")
        << "Undo transaction saved: moves="
        << transaction.moves.size()
        << " createdFolders="
        << transaction.createdFolderIds.size()
        << " removedSourceFolders="
        << transaction.removedSourceFolders.size()
        << LL_ENDL;
}

bool CTSUndoManager::canUndo() const
{
    return mHasTransaction;
}

void CTSUndoManager::clear()
{
    mHasTransaction = false;
    mLastTransaction = CTSUndoTransaction();
}

const CTSUndoTransaction& CTSUndoManager::lastTransaction() const
{
    return mLastTransaction;
}

void CTSUndoManager::undo(completion_func_t completion)
{
    CTSUndoResult result;

    if (!canUndo())
    {
        result.message = "Der er ingen gemt handling at fortryde.";
        if (completion) completion(result);
        return;
    }

    for (const CTSRemovedSourceFolder& removed :
         mLastTransaction.removedSourceFolders)
    {
        LLViewerInventoryCategory* folder =
            gInventory.getCategory(removed.folderId);

        if (!folder)
        {
            ++result.foldersKept;
            continue;
        }

        if (folder->getParentUUID() == removed.originalParentId)
        {
            ++result.foldersRestored;
            continue;
        }

        gInventory.changeCategoryParent(
            folder,
            removed.originalParentId,
            false);

        LLViewerInventoryCategory* updated =
            gInventory.getCategory(removed.folderId);

        if (updated &&
            updated->getParentUUID() == removed.originalParentId)
            ++result.foldersRestored;
        else
            ++result.foldersKept;
    }

    for (auto it = mLastTransaction.moves.rbegin();
         it != mLastTransaction.moves.rend();
         ++it)
    {
        LLViewerInventoryItem* item = gInventory.getItem(it->itemId);

        if (!item)
        {
            ++result.itemsSkipped;
            continue;
        }

        if (item->getParentUUID() == it->originalParentId)
        {
            ++result.itemsRestored;
            continue;
        }

        if (item->getParentUUID() != it->newParentId)
        {
            ++result.itemsSkipped;
            continue;
        }

        gInventory.changeItemParent(
            item,
            it->originalParentId,
            false);

        LLViewerInventoryItem* updated =
            gInventory.getItem(it->itemId);

        if (updated &&
            updated->getParentUUID() == it->originalParentId)
            ++result.itemsRestored;
        else
            ++result.itemsSkipped;
    }

    for (auto it = mLastTransaction.createdFolderIds.rbegin();
         it != mLastTransaction.createdFolderIds.rend();
         ++it)
    {
        LLViewerInventoryCategory* category =
            gInventory.getCategory(*it);

        if (!category)
            continue;

        if (category->getParentUUID() != mLastTransaction.productFolderId)
        {
            ++result.foldersKept;
            continue;
        }

        LLInventoryModel::cat_array_t* childCategories = nullptr;
        LLInventoryModel::item_array_t* childItems = nullptr;

        gInventory.getDirectDescendentsOf(
            category->getUUID(),
            childCategories,
            childItems);

        const bool hasCategories =
            childCategories && !childCategories->empty();
        const bool hasItems =
            childItems && !childItems->empty();

        if (hasCategories || hasItems)
        {
            ++result.foldersKept;
            continue;
        }

        gInventory.removeCategory(category->getUUID());
        ++result.foldersRemoved;
    }

    result.success =
        result.itemsSkipped == 0 &&
        result.itemsRestored == mLastTransaction.moves.size();

    std::ostringstream msg;
    msg << "Undo: " << result.itemsRestored << " items gendannet";

    if (result.foldersRestored)
        msg << ", " << result.foldersRestored << " originale mapper gendannet";

    if (result.foldersRemoved)
        msg << ", " << result.foldersRemoved << " Assistant-mapper fjernet";

    if (result.itemsSkipped)
        msg << ", " << result.itemsSkipped << " items sprunget over";

    if (result.foldersKept)
        msg << ", " << result.foldersKept << " mapper beholdt";

    msg << ".";
    result.message = msg.str();

    if (result.success)
        clear();

    if (completion)
        completion(result);
}
