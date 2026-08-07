#ifndef CTS_PRODUCT_ANALYSIS_H
#define CTS_PRODUCT_ANALYSIS_H

#include <set>
#include <string>

#include "ctsplan.h"

struct CTSProductAnalysis
{
    std::set<std::string> subfolders;
    std::set<std::string> colors;
    std::set<std::string> variants;
    std::set<std::string> sides;
    std::set<std::string> types;

    std::size_t itemCount = 0;
};

class CTSProductAnalyzer
{
public:
    static CTSProductAnalysis analyze(const CTSPlan& plan);
};

#endif
