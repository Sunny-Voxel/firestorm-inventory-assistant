#ifndef CTS_DESTINATION_PLANNER_H
#define CTS_DESTINATION_PLANNER_H

#include "fsinventoryscan.h"
#include "ctsitemclassifier.h"

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
};

#endif
