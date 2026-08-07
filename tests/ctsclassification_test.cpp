#include <iostream>
#include <string>
#include <vector>

#include "../indra/newview/fsinventorytools/ctsrules.h"

int main()
{
    struct Test
    {
        std::string item;
        std::string destination;
        std::string body;
        std::string type;
        std::string variant;
        std::string side;
        std::string color;
    };

    std::vector<Test> tests =
    {
        {
            "BikiniTop02 - Navy / Lara Petite",
            "Lara Petite",
            "Lara Petite",
            "Top",
            "",
            "",
            "Navy"
        },
        {
            "BODY,XTRA SHINE,LEGACY",
            "Legacy",
            "Legacy",
            "Body",
            "Xtra Shine",
            "",
            ""
        },
        {
            "HEELS,XTRA SHINE,REBORN",
            "Reborn",
            "Reborn",
            "Heels",
            "Xtra Shine",
            "",
            ""
        },
        {
            "Shoe Left LaraX",
            "LaraX",
            "LaraX",
            "",
            "",
            "Left",
            ""
        },
        {
            "Shoe Right LaraX",
            "LaraX",
            "LaraX",
            "",
            "",
            "Right",
            ""
        },
        {
            "Bra Black Legacy Perky",
            "Legacy Perky",
            "Legacy Perky",
            "Top",
            "",
            "",
            "Black"
        },
        {
            "Color HUD",
            "HUD",
            "",
            "",
            "",
            "",
            ""
        },
        {
            "Landmark",
            "Add On",
            "",
            "",
            "",
            "",
            ""
        }
    };

    bool ok = true;

    for (const Test& test : tests)
    {
        CTSClassification result = CTSRules::analyzeItem(test.item);

        bool passed =
            result.destination == test.destination &&
            result.body == test.body &&
            result.type == test.type &&
            result.variant == test.variant &&
            result.side == test.side &&
            result.color == test.color;

        std::cout << "\n" << test.item << "\n";
        std::cout << "  Destination: " << result.destination << "\n";
        std::cout << "  Body:        " << result.body << "\n";
        std::cout << "  Type:        " << result.type << "\n";
        std::cout << "  Variant:     " << result.variant << "\n";
        std::cout << "  Side:        " << result.side << "\n";
        std::cout << "  Color:       " << result.color << "\n";

        if (passed)
        {
            std::cout << "  [OK]\n";
        }
        else
        {
            std::cout << "  [FEJL]\n";
            ok = false;
        }
    }

    std::cout << "\n";

    if (ok)
    {
        std::cout << "Alle classification-tests bestået.\n";
        return 0;
    }

    std::cout << "Der er fejl i classification-reglerne.\n";
    return 1;
}
