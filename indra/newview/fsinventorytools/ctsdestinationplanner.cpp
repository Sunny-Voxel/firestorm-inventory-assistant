#include "../llviewerprecompiledheaders.h"

#include "ctsdestinationplanner.h"
#include "ctsrules.h"

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

    bool equalsIgnoreCase(const std::string& a, const std::string& b)
    {
        return lowerCopy(a) == lowerCopy(b);
    }

    std::string findExistingFolder(
        const std::vector<std::string>& folders,
        const std::string& wanted)
    {
        for (const std::string& folder : folders)
        {
            if (equalsIgnoreCase(folder, wanted))
                return folder;
        }
        return std::string();
    }
}

CTSDestinationPlan CTSDestinationPlanner::build(
    const FSInventoryScanResult& scan)
{
    CTSDestinationPlan plan;
    std::set<std::string> garmentTypes;

    for (const FSInventoryScannedItem& item : scan.items)
    {
        CTSClassification c = CTSRules::analyzeItem(item.itemName);

        if (!c.body.empty() &&
            !c.type.empty() &&
            c.destination != "HUD" &&
            c.destination != "Add On")
        {
            garmentTypes.insert(c.type);
        }
    }

    const bool usePartFolders = garmentTypes.size() > 1;

    for (const FSInventoryScannedItem& item : scan.items)
    {
        CTSClassification c = CTSRules::analyzeItem(item.itemName);

        CTSDestinationPlannedItem planned;
        planned.item = item;
        planned.body = c.body;
        planned.type = c.type;
        planned.color = c.color;
        planned.variant = c.variant;
        planned.side = c.side;

        planned.destination = chooseDestination(
            item, c.body, c.type, usePartFolders,
            scan.existingFolderNames);

        plan[planned.destination].push_back(planned);
    }

    return plan;
}

std::string CTSDestinationPlanner::chooseDestination(
    const FSInventoryScannedItem& item,
    const std::string& body,
    const std::string& type,
    bool usePartFolders,
    const std::vector<std::string>& existingFolders)
{
    if (item.destination == "HUD" || item.destination == "Add On")
    {
        std::string existing =
            findExistingFolder(existingFolders, item.destination);

        return existing.empty() ? item.destination : existing;
    }

    if (body.empty())
        return item.destination;

    if (!item.sourceFolderName.empty())
    {
        const std::string sourceLower = lowerCopy(item.sourceFolderName);
        const std::string bodyLower = lowerCopy(body);

        if (sourceLower == bodyLower ||
            sourceLower.rfind(bodyLower + " ", 0) == 0)
        {
            return item.sourceFolderName;
        }
    }

    if (usePartFolders && !type.empty())
    {
        const std::string wanted = body + " " + type;

        std::string existing =
            findExistingFolder(existingFolders, wanted);

        if (!existing.empty())
            return existing;

        for (const std::string& folder : existingFolders)
        {
            const std::string folderLower = lowerCopy(folder);

            if (folderLower.rfind(lowerCopy(body) + " ", 0) == 0)
            {
                if ((type == "Tong" &&
                     (folderLower.find("tong") != std::string::npos ||
                      folderLower.find("thong") != std::string::npos)) ||
                    (type == "Top" &&
                     folderLower.find("top") != std::string::npos))
                {
                    return folder;
                }
            }
        }

        return wanted;
    }

    std::string existing = findExistingFolder(existingFolders, body);
    return existing.empty() ? body : existing;
}
