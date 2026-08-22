#include <iostream>
#include <string>

#include "../indra/newview/fsinventorytools/ctsrules.h"

int main()
{
    struct Test
    {
        std::string item;
        std::string expected;
    };

    Test tests[] =
    {
        {"BikiniTop02 - Black / Lara", "Lara"},
        {"Dress LaraX",          "LaraX"},
        {"Corset Lara X",        "LaraX"},
        {"Top LaraX Petite",     "LaraX Petite"},
        {"Jeans Legacy",         "Legacy"},
        {"Dress Legacy Perky",   "Legacy Perky"},
        {"Top Reborn",           "Reborn"},
        {"Dress Reborn Waifu",   "Reborn Waifu"},
        {"Body Kupra",           "Kupra"},
        {"Dress Belleza",        "Belleza"},
        {"Top Gen.X Curvy",      "GenX Curvy"},
        {"HUD",                  "HUD"},
        {"Read Me",              "Add On"},
        {"Landmark",             "Add On"}
    };

    bool ok = true;

    for (const auto& test : tests)
    {
        std::string result = CTSRules::classifyItem(test.item);

        std::cout
            << test.item
            << " -> "
            << result;

        if (result == test.expected)
        {
            std::cout << "  [OK]";
        }
        else
        {
            std::cout
                << "  [FEJL - forventede "
                << test.expected
                << "]";
            ok = false;
        }

        std::cout << std::endl;
    }

    if (ok)
    {
        std::cout << std::endl;
        std::cout << "Alle tests bestået." << std::endl;
        return 0;
    }

    std::cout << std::endl;
    std::cout << "Der er fejl i reglerne." << std::endl;

    return 1;
}
