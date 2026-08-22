#ifndef FS_INVENTORY_ADAPTER_H
#define FS_INVENTORY_ADAPTER_H

#include "lluuid.h"

class FSInventoryAdapter
{
public:
    static void analyzeFolder(const LLUUID& folderId);
};

#endif
