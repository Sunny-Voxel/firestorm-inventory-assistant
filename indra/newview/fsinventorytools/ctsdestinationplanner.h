#ifndef CTS_DESTINATION_PLANNER_H
#define CTS_DESTINATION_PLANNER_H

#include "fsinventoryscan.h"

#include <map>
#include <string>
#include <vector>

struct CTSDestinationPlannedItem
{
    FSInventoryScannedItem item;
    std::string destination;
    std::string body;
    std::string type;
    std::string color;
    std::string variant;
    std::string side;
};

using CTSDestinationPlan =
    std::map<std::string, std::vector<CTSDestinationPlannedItem>>;

class CTSDestinationPlanner
{
public:
    static CTSDestinationPlan build(const FSInventoryScanResult& scan);

private:
    static std::string chooseDestination(
        const FSInventoryScannedItem& item,
        const std::string& body,
        const std::string& type,
        bool usePartFolders,
        const std::vector<std::string>& existingFolders);
};

#endif
