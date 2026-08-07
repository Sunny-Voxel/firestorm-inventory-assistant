#ifndef CTS_PLAN_H
#define CTS_PLAN_H

#include <map>
#include <string>
#include <vector>

#include "ctsclassification.h"

struct CTSPlannedItem
{
    std::string name;
    CTSClassification classification;
};

using CTSPlan = std::map<std::string, std::vector<CTSPlannedItem>>;

class CTSPlanBuilder
{
public:
    static CTSPlan buildPlan(const std::vector<std::string>& itemNames);
};

#endif
