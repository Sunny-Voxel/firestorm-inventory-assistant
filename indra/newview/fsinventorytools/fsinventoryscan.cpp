#include "../llviewerprecompiledheaders.h"

#include "fsinventoryscan.h"

#include "../llinventorymodel.h"
#include "../llviewerinventory.h"

#include "ctsrules.h"

namespace
{
    void scanFolderItems(
        const LLUUID& folderId,
        const std::string& folderName,
        FSInventoryScanResult& result)
    {
        LLInventoryModel::cat_array_t* categories = nullptr;
        LLInventoryModel::item_array_t* items = nullptr;

        gInventory.getDirectDescendentsOf(
            folderId,
            categories,
            items);

        if (!items)
        {
            return;
        }

        for (const auto& item : *items)
        {
            if (!item)
            {
                continue;
            }

            FSInventoryScannedItem scanned;

            scanned.itemId = item->getUUID();
            scanned.sourceFolderId = folderId;
            scanned.itemName = item->getName();
            scanned.sourceFolderName = folderName;
            scanned.destination =
                CTSRules::classifyItem(scanned.itemName);

            result.items.push_back(scanned);
        }
    }
}

FSInventoryScanResult FSInventoryScanner::scanProductFolder(
    const LLUUID& folderId)
{
    FSInventoryScanResult result;
    result.productFolderId = folderId;

    const LLViewerInventoryCategory* product =
        gInventory.getCategory(folderId);

    result.productName =
        product ? product->getName() : folderId.asString();

    // Items direkte i produktmappen.
    scanFolderItems(
        folderId,
        result.productName,
        result);

    // Scan kun ét niveau ned.
    LLInventoryModel::cat_array_t* categories = nullptr;
    LLInventoryModel::item_array_t* items = nullptr;

    gInventory.getDirectDescendentsOf(
        folderId,
        categories,
        items);

    if (categories)
    {
        for (const auto& category : *categories)
        {
            if (!category)
            {
                continue;
            }

            result.existingFolderNames.push_back(category->getName());

            scanFolderItems(
                category->getUUID(),
                category->getName(),
                result);
        }
    }

    return result;
}
