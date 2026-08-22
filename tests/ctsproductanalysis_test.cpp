#include <iostream>
#include <string>
#include <vector>

#include "../indra/newview/fsinventorytools/ctsplan.h"
#include "../indra/newview/fsinventorytools/ctsproductanalysis.h"

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
    CTSProductAnalysis analysis = CTSProductAnalyzer::analyze(plan);

    std::cout << "Items: " << analysis.itemCount << "\n";

    std::cout << "\nSubfolders:\n";
    for (const std::string& value : analysis.subfolders)
        std::cout << "  - " << value << "\n";

    std::cout << "\nColors:\n";
    for (const std::string& value : analysis.colors)
        std::cout << "  - " << value << "\n";

    std::cout << "\nVariants:\n";
    for (const std::string& value : analysis.variants)
        std::cout << "  - " << value << "\n";

    std::cout << "\nSides:\n";
    for (const std::string& value : analysis.sides)
        std::cout << "  - " << value << "\n";

    std::cout << "\nTypes:\n";
    for (const std::string& value : analysis.types)
        std::cout << "  - " << value << "\n";

    return 0;
}
