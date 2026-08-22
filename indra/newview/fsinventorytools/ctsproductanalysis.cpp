#include "ctsproductanalysis.h"

CTSProductAnalysis CTSProductAnalyzer::analyze(const CTSPlan& plan)
{
    CTSProductAnalysis result;

    for (const auto& group : plan)
    {
        const std::string& destination = group.first;

        if (!destination.empty())
        {
            result.subfolders.insert(destination);
        }

        for (const CTSPlannedItem& item : group.second)
        {
            ++result.itemCount;

            const CTSClassification& c = item.classification;

            if (!c.color.empty())
            {
                result.colors.insert(c.color);
            }

            if (!c.variant.empty())
            {
                result.variants.insert(c.variant);
            }

            if (!c.side.empty())
            {
                result.sides.insert(c.side);
            }

            if (!c.type.empty())
            {
                result.types.insert(c.type);
            }
        }
    }

    return result;
}
