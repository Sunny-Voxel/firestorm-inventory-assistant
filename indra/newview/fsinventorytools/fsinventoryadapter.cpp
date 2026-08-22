#include "../llviewerprecompiledheaders.h"

#include "fsinventoryadapter.h"

#include "llnotificationsutil.h"

#include "fsinventoryscan.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

void FSInventoryAdapter::analyzeFolder(const LLUUID& folderId)
{
    FSInventoryScanResult scan =
        FSInventoryScanner::scanProductFolder(folderId);

    std::map<std::string, std::vector<FSInventoryScannedItem>> plan;

    for (const FSInventoryScannedItem& item : scan.items)
    {
        plan[item.destination].push_back(item);
    }

    LL_INFOS("FSInventoryTools")
        << "Analyzing folder: "
        << scan.productName
        << LL_ENDL;

    LL_INFOS("FSInventoryTools")
        << "Items: "
        << scan.items.size()
        << LL_ENDL;

    std::ostringstream preview;

    preview << "CTS Organizer\n\n";
    preview << scan.productName << "\n";
    preview << "Items fundet: " << scan.items.size() << "\n\n";

    std::size_t changes = 0;

    for (const auto& group : plan)
    {
        preview << group.first
                << " (" << group.second.size() << ")\n";

        for (const FSInventoryScannedItem& item : group.second)
        {
            bool alreadyCorrect =
                item.sourceFolderName == item.destination;

            bool inProductRoot =
                item.sourceFolderId == scan.productFolderId;

            preview << "  - " << item.itemName;

            if (alreadyCorrect)
            {
                preview << "  [OK]";
            }
            else if (inProductRoot)
            {
                preview << "\n      Produktmappe -> "
                        << item.destination;
                ++changes;
            }
            else
            {
                preview << "\n      "
                        << item.sourceFolderName
                        << " -> "
                        << item.destination;
                ++changes;
            }

            preview << "\n";
        }

        preview << "\n";
    }

    preview << "Foreslåede flytninger: "
            << changes
            << "\n";

    if (changes == 0)
    {
        preview << "\nStrukturen ser allerede korrekt ud.";
    }
    else
    {
        preview << "\nDer er endnu ikke flyttet noget.";
    }

    LLSD args;
    args["MESSAGE"] = preview.str();

    LLNotificationsUtil::add(
        "GenericAlert",
        args);
}
