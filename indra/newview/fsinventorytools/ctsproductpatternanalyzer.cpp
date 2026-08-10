#include "../llviewerprecompiledheaders.h"

#include "ctsproductpatternanalyzer.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct Token
{
    std::string key;
    std::string display;
};

std::string lowerCopy(std::string value)
{
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });
    return value;
}

std::string titleToken(const std::string& value)
{
    if (value.empty())
        return value;

    bool allDigits = true;
    for (unsigned char c : value)
    {
        if (!std::isdigit(c))
        {
            allDigits = false;
            break;
        }
    }

    if (allDigits)
        return value;

    std::string out = lowerCopy(value);
    out[0] = static_cast<char>(
        std::toupper(static_cast<unsigned char>(out[0])));

    if (out == "Hud") return "HUD";
    if (out == "Xtra") return "Xtra";

    return out;
}

std::vector<Token> splitKnownBodySuffix(const Token& token)
{
    const std::vector<std::pair<std::string, std::string>> suffixes =
    {
        {"maitreya", "Maitreya"},
        {"legacy", "Legacy"},
        {"reborn", "Reborn"},
        {"kupra", "Kupra"},
        {"erika", "Erika"},
        {"vtech", "VTech"},
        {"freya", "Freya"}
    };

    for (const auto& suffix : suffixes)
    {
        if (token.key.size() <= suffix.first.size())
            continue;

        const std::size_t start =
            token.key.size() - suffix.first.size();

        if (token.key.compare(
                start,
                suffix.first.size(),
                suffix.first) != 0)
        {
            continue;
        }

        Token prefix;
        prefix.key = token.key.substr(0, start);
        prefix.display = titleToken(prefix.key);

        Token body;
        body.key = suffix.first;
        body.display = suffix.second;

        std::vector<Token> result;
        if (!prefix.key.empty())
            result.push_back(prefix);
        result.push_back(body);
        return result;
    }

    return {token};
}

std::vector<Token> tokenize(const std::string& value)
{
    std::vector<Token> result;
    std::string current;
    unsigned char previous = 0;

    auto flush = [&]()
    {
        if (current.empty())
            return;

        Token token;
        token.key = lowerCopy(current);
        token.display = titleToken(current);
        const std::vector<Token> expanded =
            splitKnownBodySuffix(token);

        result.insert(
            result.end(),
            expanded.begin(),
            expanded.end());

        current.clear();
    };

    for (std::size_t i = 0; i < value.size(); ++i)
    {
        const unsigned char c =
            static_cast<unsigned char>(value[i]);

        if (std::isalnum(c))
        {
            if (!current.empty() && previous != 0)
            {
                const bool letterToDigit =
                    std::isalpha(previous) &&
                    std::isdigit(c);

                const bool digitToLetter =
                    std::isdigit(previous) &&
                    std::isalpha(c);

                // Also split simple CamelCase: BikiniTop -> Bikini / Top.
                const bool camelBoundary =
                    std::islower(previous) &&
                    std::isupper(c);

                if (letterToDigit ||
                    digitToLetter ||
                    camelBoundary)
                {
                    flush();
                }
            }

            current.push_back(
                static_cast<char>(c));
            previous = c;
        }
        else
        {
            flush();

            if (c == '%' &&
                !result.empty())
            {
                result.back().key += "%";
                result.back().display += "%";
            }

            previous = 0;
        }
    }

    flush();
    return result;
}

std::set<std::string> bodyAliasTokens(
    const std::string& body)
{
    std::set<std::string> result;

    auto add = [&](const std::string& value)
    {
        for (const Token& token : tokenize(value))
            result.insert(token.key);
    };

    add(body);

    if (body == "Lara")
    {
        add("Maitreya");
        add("Maitr");
    }
    else if (body == "Lara X")
    {
        add("Maitreya Lara X");
    }
    else if (body == "SLink Hourglass")
    {
        add("SLink H");
        add("Hourglass");
    }
    else if (body == "SLink Physique")
    {
        add("SLink P");
        add("Physique");
    }
    else if (body == "Freya")
    {
        add("B Freya");
        add("Belleza Freya");
    }

    return result;
}

bool genericNoiseToken(const std::string& token)
{
    static const std::set<std::string> noise =
    {
        "triggered",
        "fatpack",
        "boxed",
        "unpacked",
        "ver",
        "version"
    };

    return noise.count(token) != 0;
}

std::string compactDescriptor(
    const std::vector<Token>& tokens)
{
    if (tokens.empty())
        return std::string();

    std::ostringstream out;

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        const bool numeric =
            !tokens[i].key.empty() &&
            std::isdigit(
                static_cast<unsigned char>(
                    tokens[i].key[0]));

        if (i > 0 && !numeric)
            out << " ";

        out << tokens[i].display;
    }

    return out.str();
}

std::string fallbackDescriptor(
    const CTSItemClassification& c)
{
    std::vector<std::string> parts;

    // "Body" itself carries no useful CTS distinction.
    if (!c.type.empty() &&
        lowerCopy(c.type) != "body")
    {
        parts.push_back(c.type);
    }

    if (!c.placement.empty() &&
        lowerCopy(c.placement) !=
            lowerCopy(c.type))
    {
        parts.push_back(c.placement);
    }

    if (!c.side.empty())
        parts.push_back(c.side);

    if (!c.variant.empty())
        parts.push_back(c.variant);

    if (!c.version.empty())
    {
        if (!parts.empty())
            parts.back() += c.version;
        else
            parts.push_back("Version" + c.version);
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < parts.size(); ++i)
    {
        if (i) out << " ";
        out << parts[i];
    }

    return out.str();
}
}

std::map<LLUUID, std::string>
CTSProductPatternAnalyzer::analyze(
    const FSInventoryScanResult& scan,
    const std::map<LLUUID, CTSItemClassification>& classifications)
{
    std::map<LLUUID, std::string> result;

    // Work body-by-body. Differences are meaningful only between
    // siblings that target the same body.
    std::map<std::string, std::vector<const FSInventoryScannedItem*>>
        groups;

    for (const FSInventoryScannedItem& item : scan.items)
    {
        const auto it =
            classifications.find(item.itemId);

        if (it == classifications.end() ||
            it->second.body.empty())
        {
            continue;
        }

        groups[it->second.body].push_back(&item);
    }

    for (const auto& groupEntry : groups)
    {
        const std::string& body = groupEntry.first;
        const auto& items = groupEntry.second;

        std::map<LLUUID, std::vector<Token>> tokensByItem;
        std::map<std::string, std::size_t> itemFrequency;

        const std::set<std::string> bodyTokens =
            bodyAliasTokens(body);

        // Tokenize every sibling and count whether each token occurs
        // in each item. Counting is per item, not raw occurrences.
        for (const FSInventoryScannedItem* item : items)
        {
            std::vector<Token> filtered;
            std::set<std::string> seenInItem;

            for (const Token& token : tokenize(item->itemName))
            {
                if (bodyTokens.count(token.key))
                    continue;

                if (genericNoiseToken(token.key))
                    continue;

                filtered.push_back(token);
                seenInItem.insert(token.key);
            }

            tokensByItem[item->itemId] = filtered;

            for (const std::string& token : seenInItem)
                ++itemFrequency[token];
        }

        const std::size_t groupSize =
            items.size();

        for (const FSInventoryScannedItem* item : items)
        {
            std::vector<Token> distinguishing;

            for (const Token& token :
                 tokensByItem[item->itemId])
            {
                const auto freq =
                    itemFrequency.find(token.key);

                if (freq != itemFrequency.end() &&
                    freq->second == groupSize)
                {
                    // Shared by every item in this body group:
                    // store/product/color context, not a distinction.
                    continue;
                }

                distinguishing.push_back(token);
            }

            // If the classifier says this is a generic BODY item,
            // BODY itself is not useful in the button name.
            const CTSItemClassification& c =
                classifications.at(item->itemId);

            distinguishing.erase(
                std::remove_if(
                    distinguishing.begin(),
                    distinguishing.end(),
                    [&](const Token& token)
                    {
                        return lowerCopy(c.type) == "body" &&
                               token.key == "body";
                    }),
                distinguishing.end());

            std::string descriptor =
                compactDescriptor(distinguishing);

            // A single item, or a group with no lexical distinction,
            // still benefits from known semantic fields.
            if (descriptor.empty())
                descriptor = fallbackDescriptor(c);

            result[item->itemId] = descriptor;
        }
    }

    return result;
}
