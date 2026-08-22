#ifndef CTS_DESTINATION_PLANNER_H
#define CTS_DESTINATION_PLANNER_H

#include "fsinventoryscan.h"
#include "ctsitemclassifier.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

struct CTSDestinationPlannedItem
{
    FSInventoryScannedItem item;
    std::string destination;
    CTSItemClassification classification;
    bool parkedInAddOn = false;
    std::string reason;
};

using CTSDestinationPlan =
    std::map<std::string, std::vector<CTSDestinationPlannedItem>>;

class CTSDestinationPlanner
{
public:
    static CTSDestinationPlan build(
        const FSInventoryScanResult& scan);

private:
    static std::string chooseDestination(
        const FSInventoryScannedItem& item,
        const CTSItemClassification& classification,
        bool splitVersion,
        bool splitVariant,
        std::size_t alphaCount,
        const std::vector<std::string>& existingFolders,
        bool& parkedInAddOn,
        std::string& reason);
};

#endif
