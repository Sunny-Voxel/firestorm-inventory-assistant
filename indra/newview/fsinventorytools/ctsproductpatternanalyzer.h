#ifndef CTS_PRODUCT_PATTERN_ANALYZER_H
#define CTS_PRODUCT_PATTERN_ANALYZER_H

#include "fsinventoryscan.h"
#include "ctsitemclassifier.h"

#include <map>
#include <string>

class CTSProductPatternAnalyzer
{
public:
    // Returns a meaningful suffix for each recognized-body item.
    // Example:
    //   Lara + BikiniTop01 -> "Top01"
    //   Lara + Fishnet     -> "Fishnet"
    //   Legacy normal      -> ""
    //   Legacy Animalistic -> "Animalistic"
    static std::map<LLUUID, std::string> analyze(
        const FSInventoryScanResult& scan,
        const std::map<LLUUID, CTSItemClassification>& classifications);
};

#endif
