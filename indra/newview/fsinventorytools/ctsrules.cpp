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

static bool containsToken(
    const std::string& value,
    const std::string& token)
{
    std::size_t pos = 0;
    while ((pos = value.find(token, pos)) != std::string::npos)
    {
        const bool leftOk =
            pos == 0 ||
            !std::isalnum(
                static_cast<unsigned char>(value[pos - 1]));

        const std::size_t end = pos + token.size();
        const bool rightOk =
            end >= value.size() ||
            !std::isalnum(
                static_cast<unsigned char>(value[end]));

        if (leftOk && rightOk)
            return true;

        ++pos;
    }

    return false;
}

std::string CTSRules::classifyItem(const std::string& itemName)
{
    const std::string name = toLower(itemName);

    // CTS_ALWAYS_ADDON_AD_0021J
    // Product advertising/preview image token: always Add On.
    if (containsToken(name, "ad"))
        return "Add On";

    // CTS_SPECIAL_SHARED_0021
    // HUD/Alpha are shared structural items, even if a body word is also present.
    if (name.find("hud") != std::string::npos)
        return "HUD";

    if (name.find("alpha") != std::string::npos)
        return "Alpha";

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

    // Mange creators bruger blot "Lara" i item-navnet.
    // De mere specifikke LaraX/Lara Petite-regler ligger ovenfor,
    // så de bliver fortsat valgt først.
    if (name.find("lara") != std::string::npos)
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
