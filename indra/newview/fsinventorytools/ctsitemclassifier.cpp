#include "../llviewerprecompiledheaders.h"

#include "ctsitemclassifier.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
std::string lowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });
    return value;
}

std::string normalized(const std::string& value)
{
    std::string out(" ");
    bool lastSpace = true;

    for (unsigned char c : value)
    {
        if (std::isalnum(c))
        {
            out.push_back(static_cast<char>(std::tolower(c)));
            lastSpace = false;
        }
        else if (!lastSpace)
        {
            out.push_back(' ');
            lastSpace = true;
        }
    }

    if (out.back() != ' ')
        out.push_back(' ');

    return out;
}

bool containsPhrase(const std::string& normalizedName, const std::string& phrase)
{
    return normalizedName.find(" " + lowerCopy(phrase) + " ") != std::string::npos;
}

void addSignal(CTSItemClassification& result, const std::string& signal, int score)
{
    result.signals.push_back(signal);
    result.score += score;
}

std::string findLooseBody(const std::string& original)
{
    const std::string value = lowerCopy(original);

    const std::vector<std::pair<std::string, std::string>> aliases =
    {
        {"maitreya lara x", "Lara X"},
        {"larax", "Lara X"},
        {"lara petite", "Lara Petite"},
        {"larapetite", "Lara Petite"},
        {"slinkhourglass", "SLink Hourglass"},
        {"slinkphysique", "SLink Physique"},
        {"maitreya", "Lara"},
        {"legacy", "Legacy"},
        {"reborn", "Reborn"},
        {"kupra", "Kupra"},
        {"erika", "Erika"},
        {"vtech", "VTech"},
        {"freya", "Freya"}
    };

    for (const auto& alias : aliases)
    {
        if (value.find(alias.first) != std::string::npos)
            return alias.second;
    }

    return std::string();
}

std::string findFirst(
    const std::string& name,
    const std::vector<std::pair<std::string, std::string>>& values)
{
    for (const auto& value : values)
    {
        if (containsPhrase(name, value.first))
            return value.second;
    }
    return std::string();
}

std::string extractVersionAfterType(
    const std::string& original,
    const std::string& type)
{
    if (type.empty())
        return std::string();

    const std::string lowerOriginal = lowerCopy(original);
    const std::string lowerType = lowerCopy(type);

    std::size_t pos = lowerOriginal.find(lowerType);
    if (pos == std::string::npos)
        return std::string();

    pos += lowerType.size();

    while (pos < original.size() &&
           (original[pos] == ' ' ||
            original[pos] == '-' ||
            original[pos] == '_' ||
            original[pos] == '.'))
    {
        ++pos;
    }

    if (pos < original.size() &&
        (original[pos] == 'v' || original[pos] == 'V'))
    {
        ++pos;
    }

    std::string digits;
    while (pos < original.size() &&
           std::isdigit(static_cast<unsigned char>(original[pos])))
    {
        digits.push_back(original[pos]);
        ++pos;
    }

    return digits;
}
std::string extractPercentage(const std::string& original)
{
    for (std::size_t i = 0; i < original.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(original[i])))
            continue;

        std::size_t j = i;
        while (j < original.size() &&
               std::isdigit(static_cast<unsigned char>(original[j])))
            ++j;

        while (j < original.size() && original[j] == ' ')
            ++j;

        if (j < original.size() && original[j] == '%')
            return original.substr(i, j - i + 1);
    }
    return std::string();
}

}

CTSItemClassification CTSItemClassifier::classify(const std::string& itemName)
{
    CTSItemClassification result;
    const std::string name = normalized(itemName);

    const std::vector<std::pair<std::string, std::string>> bodies =
    {
        {"maitreya lara x", "Lara X"},
        {"lara x", "Lara X"},
        {"lara petite", "Lara Petite"},
        {"slink h petite", "SLink Hourglass"},
        {"slink p petite", "SLink Physique"},
        {"slink hourglass", "SLink Hourglass"},
        {"slink physique", "SLink Physique"},
        {"slink h", "SLink Hourglass"},
        {"slink p", "SLink Physique"},
        {"b freya", "Freya"},
        {"vtech", "VTech"},
        {"gen x", "Gen X"},
        {"legacy", "Legacy"},
        {"reborn", "Reborn"},
        {"freya", "Freya"},
        {"hourglass", "Hourglass"},
        {"physique", "Physique"},
        {"kupra", "Kupra"},
        {"maitreya", "Lara"},
        {"maitr", "Lara"},
        {"lara", "Lara"}
    };

    result.body = findFirst(name, bodies);

    if (result.body.empty())
        result.body = findLooseBody(itemName);

    if (!result.body.empty())
        addSignal(result, "body=" + result.body, 35);

    const std::vector<std::pair<std::string, std::string>> types =
    {
        {"body", "Body"},
        {"bikini thong", "Thong"},
        {"bikinithong", "Thong"},
        {"thong", "Thong"},
        {"tong", "Thong"},
        {"panties", "Panties"},
        {"panty", "Panties"},
        {"bikini top", "Top"},
        {"bikinitop", "Top"},
        {"top", "Top"},
        {"bottom", "Bottom"},
        {"bra", "Bra"},
        {"dress", "Dress"},
        {"skirt", "Skirt"},
        {"shorts", "Shorts"},
        {"pants", "Pants"},
        {"trousers", "Pants"},
        {"leggings", "Leggings"},
        {"stockings", "Stockings"},
        {"socks", "Socks"},
        {"boots", "Boots"},
        {"heels", "Heels"},
        {"shoes", "Shoes"},
        {"gloves", "Gloves"},
        {"glove", "Gloves"},
        {"bracelet", "Bracelet"},
        {"cuff", "Cuff"},
        {"ring", "Ring"},
        {"necklace", "Necklace"},
        {"collar", "Collar"},
        {"tattoo", "Tattoo"},
        {"strap", "Strap"}
    };

    result.type = findFirst(name, types);
    if (!result.type.empty())
        addSignal(result, "type=" + result.type, 30);

    const std::vector<std::pair<std::string, std::string>> placements =
    {
        {"left wrist", "Wrist"},
        {"right wrist", "Wrist"},
        {"left ankle", "Ankle"},
        {"right ankle", "Ankle"},
        {"left thigh", "Thigh"},
        {"right thigh", "Thigh"},
        {"left arm", "Arm"},
        {"right arm", "Arm"},
        {"left hand", "Hand"},
        {"right hand", "Hand"},
        {"left foot", "Foot"},
        {"right foot", "Foot"},
        {"wrist", "Wrist"},
        {"ankle", "Ankle"},
        {"thigh", "Thigh"},
        {"forearm", "Arm"},
        {"upper arm", "Arm"},
        {"arm", "Arm"},
        {"hand", "Hand"},
        {"foot", "Foot"},
        {"leg", "Leg"},
        {"neck", "Neck"},
        {"head", "Head"},
        {"chest", "Chest"},
        {"waist", "Waist"},
        {"hip", "Hip"},
        {"ear", "Ear"}
    };

    result.placement = findFirst(name, placements);
    if (!result.placement.empty())
        addSignal(result, "placement=" + result.placement, 25);

    if (containsPhrase(name, "left"))
    {
        result.side = "Left";
        addSignal(result, "side=Left", 15);
    }
    else if (containsPhrase(name, "right"))
    {
        result.side = "Right";
        addSignal(result, "side=Right", 15);
    }
    else if (containsPhrase(name, "both"))
    {
        result.side = "Both";
        addSignal(result, "side=Both", 10);
    }

    // Semantic variants. Most specific phrases must come first.
    const std::vector<std::pair<std::string, std::string>> variants =
    {
        {"animalistic ver", "Animalistic"},
        {"animalistic", "Animalistic"},
        {"xtra shine", "Xtra Shine"},
        {"extra shine", "Xtra Shine"},
        {"without piercing", "Without Piercing"},
        {"w/o piercing", "Without Piercing"},
        {"wo piercing", "Without Piercing"},
        {"piercing only", "Piercing Only"},
        {"with piercing", "Piercing"},
        {"piercing", "Piercing"},
        {"medium", "Medium"},
        {"dark", "Dark"},
        {"light", "Light"},
        {"ballet", "Ballet"},
        {"flat", "Flat"},
        {"high", "High"},
        {"loose", "Loose"},
        {"tight", "Tight"},
        {"petite", "Petite"},
        {"perky", "Perky"},
        {"waifu", "Waifu"},
        {"pudge", "Pudge"},
        {"push up", "Push Up"},
        {"pushup", "Push Up"},
        {"juicy", "Juicy"},
        {"open", "Open"},
        {"closed", "Closed"}
    };

    result.variant = findFirst(name, variants);

    const std::string percentage = extractPercentage(itemName);
    if (!percentage.empty())
    {
        result.variant = percentage;
        addSignal(result, "variant=" + result.variant, 20);
    }
    else if (!result.variant.empty())
    {
        addSignal(result, "variant=" + result.variant, 15);
    }

    result.version = extractVersionAfterType(itemName, result.type);
    if (!result.version.empty())
        addSignal(result, "version=" + result.version, 10);

    const std::vector<std::pair<std::string, std::string>> colors =
    {
        {"black", "Black"}, {"white", "White"}, {"red", "Red"},
        {"blue", "Blue"}, {"green", "Green"}, {"pink", "Pink"},
        {"purple", "Purple"}, {"yellow", "Yellow"}, {"brown", "Brown"},
        {"tortoise", "Tortoise"},
        {"grey", "Grey"}, {"gray", "Grey"}, {"orange", "Orange"}
    };

    result.color = findFirst(name, colors);
    if (!result.color.empty())
        addSignal(result, "color=" + result.color, 5);

    if (result.score >= 60)
        result.confidence = CTSConfidence::High;
    else if (result.score >= 35)
        result.confidence = CTSConfidence::Medium;
    else
        result.confidence = CTSConfidence::Low;

    return result;
}

const char* CTSItemClassifier::confidenceName(CTSConfidence confidence)
{
    switch (confidence)
    {
        case CTSConfidence::High: return "High";
        case CTSConfidence::Medium: return "Medium";
        default: return "Low";
    }
}
