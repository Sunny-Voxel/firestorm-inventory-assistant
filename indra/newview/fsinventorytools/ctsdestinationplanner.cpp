#include "../llviewerprecompiledheaders.h"

#include "ctsdestinationplanner.h"
#include "ctsbodysettings.h"
#include "ctsproductpatternanalyzer.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>

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
}

CTSDestinationPlan CTSDestinationPlanner::build(
    const FSInventoryScanResult& scan)
{
    std::map<LLUUID, CTSItemClassification>
        classifications;

    for (const FSInventoryScannedItem& item :
         scan.items)
    {
        classifications[item.itemId] =
            CTSItemClassifier::classify(
                item.itemName);
    }

    const std::map<LLUUID, std::string>
        patternDescriptors =
            CTSProductPatternAnalyzer::analyze(
                scan,
                classifications);

    // CTS_COLOR_VARIANTS_0020C
    std::size_t colorVariantCount = 0;
    for (const auto& entry : classifications)
    {
        if (entry.second.body.empty() &&
            !entry.second.color.empty())
        {
            ++colorVariantCount;
        }
    }
    const bool useColorVariants = colorVariantCount >= 2;

    CTSDestinationPlan plan;

    for (const FSInventoryScannedItem& item :
         scan.items)
    {
        CTSDestinationPlannedItem planned;
        planned.item = item;
        planned.classification =
            classifications[item.itemId];

        const CTSItemClassification& c =
            planned.classification;

        // HUD is always a fixed destination.
        if (item.destination == "HUD")
        {
            planned.destination =
                findExistingFolder(
                    scan.existingFolderNames,
                    "HUD");

            if (planned.destination.empty())
                planned.destination = "HUD";

            planned.reason = "HUD";
            plan[planned.destination]
                .push_back(planned);
            continue;
        }

        // CTS_FORCE_ADDON_0021L
        // Items explicitly classified as Add On by CTSRules must never
        // create semantic folders. This includes product AD/preview images.
        if (item.destination == "Add On" &&
            c.body.empty() &&
            c.head.empty() &&
            c.otherTarget.empty())
        {
            planned.destination =
                findExistingFolder(
                    scan.existingFolderNames,
                    "Add On");

            if (planned.destination.empty())
                planned.destination = "Add On";

            planned.parkedInAddOn = true;
            planned.reason = "Explicit Add On rule";

            plan[planned.destination]
                .push_back(planned);
            continue;
        }

        // Bodies, heads and optional Other targets are equal
        // structural targets.
        const std::string structuralTarget =
            !c.body.empty()
                ? c.body
                : (!c.head.empty()
                    ? c.head
                    : c.otherTarget);

        // A recognized but disabled target always stays in Add On.
        if (!structuralTarget.empty() &&
            !CTSBodySettings::isTargetEnabled(structuralTarget))
        {
            planned.destination =
                findExistingFolder(
                    scan.existingFolderNames,
                    "Add On");

            if (planned.destination.empty())
                planned.destination = "Add On";

            planned.parkedInAddOn = true;
            planned.reason =
                "Target disabled: " + structuralTarget;

            plan[planned.destination]
                .push_back(planned);
            continue;
        }

        // Recognized + enabled body/head: use the target as the base
        // and append what distinguishes sibling items.
        if (!structuralTarget.empty())
        {
            std::string wanted = structuralTarget;

            const auto descriptor =
                patternDescriptors.find(
                    item.itemId);

            if (descriptor !=
                    patternDescriptors.end() &&
                !descriptor->second.empty())
            {
                wanted += " " +
                    descriptor->second;
            }

            planned.destination =
                findExistingFolder(
                    scan.existingFolderNames,
                    wanted);

            if (planned.destination.empty())
                planned.destination = wanted;

            planned.reason =
                "Product pattern";

            plan[planned.destination]
                .push_back(planned);
            continue;
        }

        if (useColorVariants && !c.color.empty())
        {
            const std::string wanted = c.color;
            planned.destination = findExistingFolder(
                scan.existingFolderNames,
                wanted);
            if (planned.destination.empty())
                planned.destination = wanted;
            planned.reason = "Product color variant";
            plan[planned.destination].push_back(planned);
            continue;
        }

        // No recognized body/head: preserve the conservative v1 behavior.
        // Unknown/readme/store items do not get invented destinations.
        if (c.confidence == CTSConfidence::Low)
        {
            planned.destination =
                findExistingFolder(
                    scan.existingFolderNames,
                    "Add On");

            if (planned.destination.empty())
                planned.destination = "Add On";

            planned.parkedInAddOn = true;
            planned.reason = "Low confidence";

            plan[planned.destination]
                .push_back(planned);
            continue;
        }

        std::string wanted;

        if (!c.type.empty())
            wanted = c.type;

        if (!c.placement.empty())
        {
            if (!wanted.empty())
                wanted += " ";
            wanted += c.placement;
        }

        if (!c.side.empty())
        {
            if (!wanted.empty())
                wanted += " ";
            wanted += c.side;
        }

        if (!c.variant.empty())
        {
            if (!wanted.empty())
                wanted += " ";
            wanted += c.variant;
        }

        if (!c.version.empty())
        {
            if (!wanted.empty())
                wanted += c.version;
            else
                wanted =
                    "Version" + c.version;
        }

        if (wanted.empty())
        {
            wanted = "Add On";
            planned.parkedInAddOn = true;
            planned.reason =
                "No structural signals";
        }
        else
        {
            planned.reason =
                "Semantic classification";
        }

        planned.destination =
            findExistingFolder(
                scan.existingFolderNames,
                wanted);

        if (planned.destination.empty())
            planned.destination = wanted;

        plan[planned.destination]
            .push_back(planned);
    }

    return plan;
}
