#include "../llviewerprecompiledheaders.h"

#include "ctsexecutionplan.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

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

bool isSpecialSharedDestination(const std::string& name)
{
    const std::string value = lowerCopy(name);

    return value == "add on" ||
           value == "hud" ||
           value == "alpha" ||
           value.rfind("alpha - ", 0) == 0;
}

std::string duplicateKey(
    const std::string& destination,
    const std::string& itemName)
{
    return lowerCopy(destination) + "\n" + itemName;
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

    std::map<LLUUID, std::string> itemDestination;

    for (const auto& group : destinationPlan)
    {
        for (const CTSDestinationPlannedItem& planned : group.second)
            itemDestination[planned.item.itemId] = group.first;
    }

    struct SourceInfo
    {
        LLUUID folderId;
        std::string oldName;
        std::set<std::string> destinations;
        std::size_t itemCount = 0;
    };

    std::map<LLUUID, SourceInfo> sources;

    for (const FSInventoryScannedItem& item : scan.items)
    {
        if (item.sourceFolderId == scan.productFolderId)
            continue;

        const auto destIt = itemDestination.find(item.itemId);
        if (destIt == itemDestination.end())
            continue;

        SourceInfo& source = sources[item.sourceFolderId];
        source.folderId = item.sourceFolderId;
        source.oldName = item.sourceFolderName;
        source.destinations.insert(destIt->second);
        ++source.itemCount;
    }

    std::map<LLUUID, std::string> renamedDestinationByFolder;
    std::set<std::string> renameTargetsLower;

    for (const auto& entry : sources)
    {
        const SourceInfo& source = entry.second;

        if (source.itemCount == 0 || source.destinations.size() != 1)
            continue;

        const std::string destination = *source.destinations.begin();

        if (lowerCopy(source.oldName) == lowerCopy(destination))
            continue;

        if (isSpecialSharedDestination(destination))
            continue;

        if (containsIgnoreCase(scan.existingFolderNames, destination))
            continue;

        const std::string targetLower = lowerCopy(destination);

        if (renameTargetsLower.count(targetLower))
            continue;

        CTSPlannedFolderRename rename;
        rename.folderId = source.folderId;
        rename.oldName = source.oldName;
        rename.newName = destination;

        result.foldersToRename.push_back(rename);
        renamedDestinationByFolder[source.folderId] = destination;
        renameTargetsLower.insert(targetLower);
    }

    struct Candidate
    {
        const CTSDestinationPlannedItem* planned = nullptr;
        bool alreadyAtDestination = false;
    };

    std::map<std::string, std::vector<Candidate>> duplicateGroups;

    for (const auto& group : destinationPlan)
    {
        const std::string& destination = group.first;

        for (const CTSDestinationPlannedItem& planned : group.second)
        {
            const FSInventoryScannedItem& item = planned.item;

            bool already =
                lowerCopy(item.sourceFolderName) == lowerCopy(destination);

            const auto renamed =
                renamedDestinationByFolder.find(item.sourceFolderId);

            if (!already &&
                renamed != renamedDestinationByFolder.end() &&
                lowerCopy(renamed->second) == lowerCopy(destination))
            {
                already = true;
            }

            Candidate candidate;
            candidate.planned = &planned;
            candidate.alreadyAtDestination = already;

            duplicateGroups[
                duplicateKey(destination, item.itemName)]
                .push_back(candidate);
        }
    }

    std::set<LLUUID> duplicateItemIds;

    for (auto& entry : duplicateGroups)
    {
        std::vector<Candidate>& candidates = entry.second;

        if (candidates.size() < 2)
            continue;

        std::size_t keeper = 0;

        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            if (candidates[i].alreadyAtDestination)
            {
                keeper = i;
                break;
            }
        }

        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            if (i == keeper)
                continue;

            const CTSDestinationPlannedItem& planned =
                *candidates[i].planned;

            CTSPlannedDuplicateRemoval duplicate;
            duplicate.itemId = planned.item.itemId;
            duplicate.sourceFolderId = planned.item.sourceFolderId;
            duplicate.itemName = planned.item.itemName;
            duplicate.sourceFolderName = planned.item.sourceFolderName;
            duplicate.destinationFolderName = planned.destination;

            result.duplicatesToRemove.push_back(duplicate);
            duplicateItemIds.insert(duplicate.itemId);
        }
    }

    std::set<std::string> foldersNeeded;

    for (const auto& group : destinationPlan)
    {
        const std::string& destination = group.first;

        bool destinationProvidedByRename = false;

        for (const CTSPlannedFolderRename& rename : result.foldersToRename)
        {
            if (lowerCopy(rename.newName) == lowerCopy(destination))
            {
                destinationProvidedByRename = true;
                break;
            }
        }

        if (!destinationProvidedByRename &&
            !containsIgnoreCase(scan.existingFolderNames, destination))
        {
            foldersNeeded.insert(destination);
        }

        for (const CTSDestinationPlannedItem& planned : group.second)
        {
            const FSInventoryScannedItem& item = planned.item;

            if (duplicateItemIds.count(item.itemId))
                continue;

            if (lowerCopy(item.sourceFolderName) == lowerCopy(destination))
            {
                ++result.alreadyCorrect;
                continue;
            }

            const auto renamed =
                renamedDestinationByFolder.find(item.sourceFolderId);

            if (renamed != renamedDestinationByFolder.end() &&
                lowerCopy(renamed->second) == lowerCopy(destination))
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
