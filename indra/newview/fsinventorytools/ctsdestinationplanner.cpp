#include "../llviewerprecompiledheaders.h"

#include "ctsdestinationplanner.h"
#include "ctsbodysettings.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
std::string lowerCopy(std::string value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });

    return value;
}

bool equalsIgnoreCase(
    const std::string& a,
    const std::string& b)
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

std::string classificationKey(
    const CTSItemClassification& c)
{
    return lowerCopy(c.body)
        + "|" + lowerCopy(c.type)
        + "|" + lowerCopy(c.placement)
        + "|" + lowerCopy(c.side);
}

std::string trimCopy(std::string value)
{
    while (!value.empty() &&
           std::isspace(
               static_cast<unsigned char>(
                   value.front())))
    {
        value.erase(value.begin());
    }

    while (!value.empty() &&
           std::isspace(
               static_cast<unsigned char>(
                   value.back())))
    {
        value.pop_back();
    }

    return value;
}

std::string extractAlphaGroup(
    const std::string& itemName)
{
    const std::size_t open =
        itemName.find('[');

    const std::size_t close =
        open == std::string::npos
            ? std::string::npos
            : itemName.find(']', open + 1);

    if (open != std::string::npos &&
        close != std::string::npos &&
        close > open + 1)
    {
        return trimCopy(
            itemName.substr(
                open + 1,
                close - open - 1));
    }

    const std::string lower =
        lowerCopy(itemName);

    const std::size_t alpha =
        lower.find("alpha");

    if (alpha == std::string::npos)
        return std::string();

    std::string tail =
        trimCopy(
            itemName.substr(alpha + 5));

    while (!tail.empty() &&
           (tail.front() == '-' ||
            tail.front() == ':' ||
            tail.front() == '/'))
    {
        tail.erase(tail.begin());
        tail = trimCopy(tail);
    }

    return tail;
}

std::string joinDestination(
    const CTSItemClassification& c,
    bool splitVersion,
    bool splitVariant)
{
    std::vector<std::string> parts;

    if (!c.body.empty())
        parts.push_back(c.body);

    if (!c.type.empty())
        parts.push_back(c.type);

    if (!c.placement.empty() &&
        (c.type.empty() ||
         !equalsIgnoreCase(
             c.type,
             c.placement)))
    {
        parts.push_back(c.placement);
    }

    if (!c.side.empty())
        parts.push_back(c.side);

    if (splitVariant &&
        !c.variant.empty())
    {
        parts.push_back(c.variant);
    }

    if (splitVersion &&
        !c.version.empty())
    {
        if (!parts.empty())
            parts.back() += c.version;
        else
            parts.push_back(
                "Version" + c.version);
    }

    if (parts.empty())
        return "Add On";

    std::ostringstream out;

    for (std::size_t i = 0;
         i < parts.size();
         ++i)
    {
        if (i)
            out << " ";

        out << parts[i];
    }

    return out.str();
}
}

CTSDestinationPlan
CTSDestinationPlanner::build(
    const FSInventoryScanResult& scan)
{
    struct Classified
    {
        const FSInventoryScannedItem* item =
            nullptr;

        CTSItemClassification classification;
    };

    std::vector<Classified> items;

    std::map<
        std::string,
        std::set<std::string>>
        versions;

    std::map<
        std::string,
        std::set<std::string>>
        variants;

    std::size_t alphaCount = 0;

    for (const FSInventoryScannedItem& item :
         scan.items)
    {
        Classified classified;
        classified.item = &item;

        classified.classification =
            CTSItemClassifier::classify(
                item.itemName);

        const std::string key =
            classificationKey(
                classified.classification);

        if (!classified.classification
                 .version.empty())
        {
            versions[key].insert(
                classified.classification
                    .version);
        }

        if (!classified.classification
                 .variant.empty())
        {
            variants[key].insert(
                classified.classification
                    .variant);
        }

        if (lowerCopy(item.itemName)
                .find("alpha") !=
            std::string::npos)
        {
            ++alphaCount;
        }

        items.push_back(classified);
    }

    CTSDestinationPlan plan;

    for (const Classified& entry :
         items)
    {
        const std::string key =
            classificationKey(
                entry.classification);

        const auto versionIt =
            versions.find(key);

        const bool splitVersion =
            versionIt != versions.end() &&
            versionIt->second.size() > 1;

        const auto variantIt =
            variants.find(key);

        const bool splitVariant =
            variantIt != variants.end() &&
            variantIt->second.size() > 1;

        CTSDestinationPlannedItem planned;
        planned.item = *entry.item;
        planned.classification =
            entry.classification;

        planned.destination =
            chooseDestination(
                *entry.item,
                entry.classification,
                splitVersion,
                splitVariant,
                alphaCount,
                scan.existingFolderNames,
                planned.parkedInAddOn,
                planned.reason);

        plan[planned.destination]
            .push_back(planned);
    }

    return plan;
}

std::string
CTSDestinationPlanner::chooseDestination(
    const FSInventoryScannedItem& item,
    const CTSItemClassification& c,
    bool splitVersion,
    bool splitVariant,
    std::size_t alphaCount,
    const std::vector<std::string>&
        existingFolders,
    bool& parkedInAddOn,
    std::string& reason)
{
    parkedInAddOn = false;
    reason.clear();

    if (item.destination == "HUD")
    {
        reason = "HUD";

        const std::string existing =
            findExistingFolder(
                existingFolders,
                "HUD");

        return existing.empty()
            ? "HUD"
            : existing;
    }

    // Alpha is a fixed special destination.
    // If several Alpha items exist, preserve
    // their own body/variant description.
    if (lowerCopy(item.itemName)
            .find("alpha") !=
        std::string::npos)
    {
        std::string wanted =
            "Alpha";

        if (alphaCount > 1)
        {
            const std::string group =
                extractAlphaGroup(
                    item.itemName);

            if (!group.empty())
                wanted += " - " + group;
        }

        reason = "Alpha";

        const std::string existing =
            findExistingFolder(
                existingFolders,
                wanted);

        return existing.empty()
            ? wanted
            : existing;
    }

    if (!c.body.empty() &&
        !CTSBodySettings::isEnabled(
            c.body))
    {
        parkedInAddOn = true;
        reason =
            "Body disabled: " + c.body;

        const std::string existing =
            findExistingFolder(
                existingFolders,
                "Add On");

        return existing.empty()
            ? "Add On"
            : existing;
    }

    if (c.confidence ==
        CTSConfidence::Low)
    {
        parkedInAddOn = true;
        reason = "Low confidence";

        const std::string existing =
            findExistingFolder(
                existingFolders,
                "Add On");

        return existing.empty()
            ? "Add On"
            : existing;
    }

    const std::string wanted =
        joinDestination(
            c,
            splitVersion,
            splitVariant);

    if (wanted == "Add On")
    {
        parkedInAddOn = true;
        reason =
            "No structural signals";
    }
    else
    {
        reason =
            std::string(
                CTSItemClassifier::
                    confidenceName(
                        c.confidence))
            + " confidence";
    }

    const std::string existing =
        findExistingFolder(
            existingFolders,
            wanted);

    return existing.empty()
        ? wanted
        : existing;
}
