#include "ctsplan.h"
#include "ctsrules.h"

CTSPlan CTSPlanBuilder::buildPlan(const std::vector<std::string>& itemNames)
{
    CTSPlan plan;

    for (const std::string& itemName : itemNames)
    {
        CTSClassification classification =
            CTSRules::analyzeItem(itemName);

        CTSPlannedItem plannedItem;
        plannedItem.name = itemName;
        plannedItem.classification = classification;

        plan[classification.destination].push_back(plannedItem);
    }

    return plan;
}
