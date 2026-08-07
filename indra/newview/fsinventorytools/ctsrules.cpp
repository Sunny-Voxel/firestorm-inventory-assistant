#include "ctsrules.h"

#include <algorithm>
#include <cctype>

static std::string toLower(std::string value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });

    return value;
}

std::string CTSRules::classifyItem(const std::string& itemName)
{
    const std::string name = toLower(itemName);

    // Mest specifikke regler først

    if (name.find("legacy perky") != std::string::npos)
        return "Legacy Perky";

    if (name.find("reborn waifu") != std::string::npos ||
        name.find("waifu") != std::string::npos)
        return "Reborn Waifu";

    if (name.find("larax petite") != std::string::npos ||
        name.find("lara x petite") != std::string::npos)
        return "LaraX Petite";

    if (name.find("larax") != std::string::npos ||
        name.find("lara x") != std::string::npos)
        return "LaraX";

    if (name.find("lara petite") != std::string::npos)
        return "Lara Petite";

    if (name.find("maitreya petite") != std::string::npos)
        return "Lara Petite";

    if (name.find("maitreya") != std::string::npos)
        return "Lara";

    if (name.find("legacy") != std::string::npos)
        return "Legacy";

    if (name.find("reborn") != std::string::npos)
        return "Reborn";

    if (name.find("kupra bimbo") != std::string::npos)
        return "Kupra Bimbo";

    if (name.find("kupra low") != std::string::npos)
        return "Kupra Low";

    if (name.find("kupra") != std::string::npos)
        return "Kupra";

    if (name.find("erika") != std::string::npos)
        return "Erika";

    if (name.find("freya") != std::string::npos)
        return "Freya";

    if (name.find("hourglass") != std::string::npos)
        return "Hourglass";

    if (name.find("belleza") != std::string::npos)
        return "Belleza";

    if (name.find("gen.x curvy") != std::string::npos ||
        name.find("genx curvy") != std::string::npos)
        return "GenX Curvy";

    if (name.find("gen.x") != std::string::npos ||
        name.find("genx") != std::string::npos)
        return "GenX";

    if (name.find("slink h petite") != std::string::npos)
        return "SLink H Petite";

    if (name.find("slink p petite") != std::string::npos)
        return "SLink P Petite";

    if (name.find("slink h") != std::string::npos)
        return "SLink H";

    if (name.find("slink p") != std::string::npos)
        return "SLink P";

    if (name.find("vtech") != std::string::npos)
        return "VTech";

    if (name.find("hud") != std::string::npos)
        return "HUD";

    return "Add On";
}

CTSClassification CTSRules::analyzeItem(const std::string& itemName)
{
    CTSClassification result;

    const std::string name = toLower(itemName);

    result.destination = classifyItem(itemName);

    if (result.destination != "HUD" &&
        result.destination != "Add On")
    {
        result.body = result.destination;
    }

    // Type
    if (name.find("heels") != std::string::npos ||
        name.find("heel") != std::string::npos)
    {
        result.type = "Heels";
    }
    else if (name.find("panties") != std::string::npos ||
             name.find("thong") != std::string::npos)
    {
        result.type = "Bottom";
    }
    else if (name.find("bra") != std::string::npos ||
             name.find("top") != std::string::npos)
    {
        result.type = "Top";
    }
    else if (name.find("body") != std::string::npos)
    {
        result.type = "Body";
    }

    // Variant
    if (name.find("xtra shine") != std::string::npos)
    {
        result.variant = "Xtra Shine";
    }

    // Side
    if (name.find("left") != std::string::npos)
    {
        result.side = "Left";
    }
    else if (name.find("right") != std::string::npos)
    {
        result.side = "Right";
    }

    // Farve
    if (name.find("black") != std::string::npos)
    {
        result.color = "Black";
    }
    else if (name.find("navy") != std::string::npos)
    {
        result.color = "Navy";
    }
    else if (name.find("white") != std::string::npos)
    {
        result.color = "White";
    }
    else if (name.find("red") != std::string::npos)
    {
        result.color = "Red";
    }
    else if (name.find("pink") != std::string::npos)
    {
        result.color = "Pink";
    }

    return result;
}
