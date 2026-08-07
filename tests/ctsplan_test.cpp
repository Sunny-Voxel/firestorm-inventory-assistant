#include <iostream>
#include <string>
#include <vector>

#include "../indra/newview/fsinventorytools/ctsplan.h"

int main()
{
    std::vector<std::string> items =
    {
        "BikiniTop02 - Navy / Lara Petite",
        "BikiniTop02 - Black / Lara Petite",
        "BODY,XTRA SHINE,LEGACY",
        "HEELS,XTRA SHINE,REBORN",
        "Shoe Left LaraX",
        "Shoe Right LaraX",
        "Bra Black Legacy Perky",
        "Color HUD",
        "Landmark",
        "Read Me"
    };

    CTSPlan plan = CTSPlanBuilder::buildPlan(items);

    for (const auto& group : plan)
    {
        std::cout << "\n[" << group.first << "]\n";

        for (const CTSPlannedItem& item : group.second)
        {
            const CTSClassification& c = item.classification;

            std::cout << "  " << item.name << "\n";

            if (!c.body.empty())
                std::cout << "    Body:    " << c.body << "\n";

            if (!c.type.empty())
                std::cout << "    Type:    " << c.type << "\n";

            if (!c.variant.empty())
                std::cout << "    Variant: " << c.variant << "\n";

            if (!c.side.empty())
                std::cout << "    Side:    " << c.side << "\n";

            if (!c.color.empty())
                std::cout << "    Color:   " << c.color << "\n";
        }
    }

    return 0;
}
