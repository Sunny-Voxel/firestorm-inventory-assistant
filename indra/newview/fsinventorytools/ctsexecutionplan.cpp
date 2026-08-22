#include "../llviewerprecompiledheaders.h"

#include "ctsexecutionplan.h"

#include <algorithm>
#include <cctype>
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

    bool containsIgnoreCase(
        const std::vector<std::string>& values,
        const std::string& wanted)
    {
        const std::string wantedLower = lowerCopy(wanted);

        for (const std::string& value : values)
        {
            if (lowerCopy(value) == wantedLower)
                return true;
        }

        return false;
    }
}

CTSExecutionPlan CTSExecutionPlanBuilder::build(
    const FSInventoryScanResult& scan,
    const CTSDestinationPlan& destinationPlan,
    CTSExecutionMode mode)
{
    CTSExecutionPlan result;
    result.mode = mode;
    result.productName = scan.productName;

    std::set<std::string> foldersNeeded;

    for (const auto& group : destinationPlan)
    {
        const std::string& destination = group.first;

        if (!containsIgnoreCase(scan.existingFolderNames, destination))
            foldersNeeded.insert(destination);

        for (const CTSDestinationPlannedItem& planned : group.second)
        {
            const FSInventoryScannedItem& item = planned.item;

            if (lowerCopy(item.sourceFolderName) == lowerCopy(destination))
            {
                ++result.alreadyCorrect;
                continue;
            }

            CTSPlannedMove move;
            move.itemId = item.itemId;
            move.sourceFolderId = item.sourceFolderId;
            move.itemName = item.itemName;
            move.sourceFolderName = item.sourceFolderName;
            move.destinationFolderName = destination;
            result.moves.push_back(move);
        }
    }

    for (const std::string& folder : foldersNeeded)
    {
        bool used = false;

        for (const CTSPlannedMove& move : result.moves)
        {
            if (lowerCopy(move.destinationFolderName) == lowerCopy(folder))
            {
                used = true;
                break;
            }
        }

        if (used)
            result.foldersToCreate.push_back({folder});
    }

    return result;
}
