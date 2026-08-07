#ifndef FS_INVENTORY_SCAN_H
#define FS_INVENTORY_SCAN_H

#include "lluuid.h"

#include <string>
#include <vector>

struct FSInventoryScannedItem
{
    LLUUID itemId;
    LLUUID sourceFolderId;

    std::string itemName;
    std::string sourceFolderName;

    std::string destination;
};

struct FSInventoryScanResult
{
    LLUUID productFolderId;
    std::string productName;

    std::vector<FSInventoryScannedItem> items;
};

class FSInventoryScanner
{
public:
    static FSInventoryScanResult scanProductFolder(const LLUUID& folderId);
};

#endif
